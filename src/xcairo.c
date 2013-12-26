/* This file is part of FEX, copyright Jesse McClure 2013 */
/* See COPYING for license information */

#include "fex.h"
#include <unistd.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>

#define HELP_FILE "/usr/share/fex/help.png"

//static const char *font = "-*-terminus-bold-*-*-*-14-*-*-*-*-*-*-*";
static const char *font = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*";

static void buttonpress(XEvent *);
static void buttonrelease(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void motionnotify(XEvent *);
static void xcairo_help();
static void zoom();

static Display *dpy;
static int scr, sw, sh, sx, sy, fftw, ffth, ffty, stride,zpy,shot_num=0;
static int review, restart, zoomer, ty, bw, wx, wy;
static Window root, win;
static Pixmap buf, pbuf, zmap;
static GC gc;
static XFontStruct *fontstruct;
static Cursor invisible_cursor, crosshair_cursor;
static Bool running, eraser;
static cairo_t *c, *l;
static cairo_surface_t *mask, *zsrc;
static float scx, scy, brushw, brushh, radius = 0.75, line_width=0.2;
static unsigned char *alphas = NULL;
static FFT *fft;
static long double ex,tex;
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]	= buttonpress,
	[ButtonRelease]	= buttonrelease,
	[Expose]		= expose,
	[KeyPress]		= keypress,
	[MotionNotify]	= motionnotify,
};

void buttonpress(XEvent *e) {
	if (eraser) {
		e->xbutton.state = Button1Mask;
		motionnotify(e);
	}
	XSync(dpy,True);
}

void buttonrelease(XEvent *e) {
	XSync(dpy,True);
	running = False;
}

void draw() {
	XCopyArea(dpy,pbuf,buf,gc,0,0,sw,sh,0,0);
	if (eraser) {
		cairo_set_source_rgba(c,1.0,0.7,0.0,0.5);
		cairo_rectangle(c,wx-brushw/2,wy-ffty-brushh/2,brushw,brushh);
		cairo_fill_preserve(c);
		cairo_set_source_rgba(c,0.9,0.6,0.0,1.0);
		cairo_stroke(c);
	}
	char str[256]; int x = bw + 4;
	if (wx > -1 && wx < fft->ts && wy > -1 && wy < fft->fs)
		sprintf(str,"time: %.3lfs | freq: %.3lfkhz | threshold: %.1fdB",
			fft->time[wx],fft->freq[wy],thresh);
	else
		sprintf(str,"time: NA | freq: NA | threshold: %.1fdB",thresh);
	XDrawString(dpy,buf,gc,x,ty,str,strlen(str));
	sprintf(str,"path: %.2Lf | time: %.2Lf | FE: %.2Lf",ex,tex,ex/tex);
	x = XTextWidth(fontstruct,str,strlen(str)) + 4;
	XDrawString(dpy,buf,gc,sw - bw - x,ty,str,strlen(str));
	x = XTextWidth(fontstruct,name,strlen(name));
	x = (sw - x)/2;
	XDrawString(dpy,buf,gc,x,ty,name,strlen(name));
	if (zoomer) {
		cairo_set_source_rgba(c,0.0,0.5,0.1,1.0);
		cairo_move_to(c,wx,0);
		cairo_line_to(c,wx,ffth);
		cairo_move_to(c,0,wy-ffty);
		cairo_line_to(c,fftw,wy-ffty);
		cairo_stroke(c);
		if (zoomer == 2) {
			cairo_rectangle(c,range[0],zpy-ffty,wx-range[0],wy-zpy);
			cairo_stroke_preserve(c);
			cairo_set_source_rgba(c,0.0,0.5,0.1,0.2);
			cairo_fill(c);
		}
	}
	XCopyArea(dpy,buf,win,gc,0,0,sw,sh,0,0);
	XFlush(dpy);
}

void expose(XEvent *e) {
	draw();
}

void keypress(XEvent *e){
	KeySym key = XkbKeycodeToKeysym(dpy,(KeyCode)e->xkey.keycode,0,0);
	unsigned int mod = (e->xkey.state&~Mod2Mask)&~LockMask;
	if (key == XK_Escape) {
		eraser = False;
		XDefineCursor(dpy,win,crosshair_cursor);
	}
	else if (mod == ControlMask) {
		if(key == XK_q) review = 0;
		else if (key == XK_r) { restart = 1; review = 0;}
		else if (key == XK_Up) thresh += 0.25;
		else if (key == XK_Down) thresh -= 0.25; 
		else if (key == XK_Right) { restart=1;  review=0; sp_floor+=1;}
		else if (key == XK_Left) { restart=1; review=0; sp_floor-=1;}
		running = False;
	}
	else if (mod == ShiftMask) {
		if (key == XK_Up) { radius += 0.05; running=False; }
		else if (key == XK_Down) { radius -= 0.05; running=False; }
		else if (key == XK_Left) { line_width -= 0.1; running=False; }
		else if (key == XK_Right) { line_width += 0.1; running=False; }
		else if (key == XK_s) {
			cairo_surface_t *pngt = cairo_xlib_surface_create(dpy,pbuf,
					DefaultVisual(dpy,scr),sw,sh);
			char *base_name = calloc(strlen(name),sizeof(char));
			char *png_name = calloc(strlen(name)+8,sizeof(char));
			strcpy(base_name,name);
			char *dot = strrchr(base_name,'.');
			if (dot) *dot = '\0';
			sprintf(png_name,"%s_%d.png",base_name,shot_num++);
			cairo_surface_write_to_png(pngt,png_name);
			cairo_surface_destroy(pngt);
			free(png_name); free(base_name);
		}
		if (radius < 0.01) radius = 0.01;
		else if (radius > 10) radius = 10;
		if (line_width < 0.01) line_width = 0.01;
		else if (line_width > 10) line_width = 10;
		cairo_set_line_width(l,line_width);
	}
	else {
		if (eraser) { /* in eraser mode */
			if (key == XK_Up) { brushw *= 1.2; brushh *=1.2; }
			else if (key == XK_Down) { brushw *= 1/1.2; brushh *=1/1.2; }
			else if (key == XK_Right) { brushw *= 1.2; brushh *=1/1.2; }
			else if (key == XK_Left) { brushw *= 1/1.2; brushh *=1.2; }
			else if (key == XK_e) {
				eraser = False;
				XDefineCursor(dpy,win,crosshair_cursor);
			}
		}
		else { /* normal mode */
			if (key == XK_z) {
				if (range[0] || range[1]) {
					memset(range,0,sizeof(range));
					restart = 1; review = 0;
				}
				else {
					zoom();
				}
			}
			else if (key == XK_h) xcairo_help();
			else if (key == XK_F1) xcairo_help();
			else if (key == XK_e) {
				Window w1, w2; int i1,i2,sx,sy; unsigned int u1;
				XQueryPointer(dpy,win,&w1,&w2,&i1,&i2,&sx,&sy,&u1);
				wx = (sx - bw)/scx;
				wy = ffth + ffty + (sy - bw)/scy;
				eraser = True;
				XDefineCursor(dpy,win,invisible_cursor);
			}
		}
	}
	draw();
}

void motionnotify(XEvent *e){
	sx = e->xbutton.x; sy = e->xbutton.y;
	wx = (sx - bw)/scx;
	wy = ffth + ffty + (sy - bw)/scy;
	if (!eraser) {draw(); return;}
	if (wx > 0 && wx < fftw && wy > ffty && wy < ffty+ffth &&
			((e->xbutton.state & Button1Mask) == Button1Mask)) {
		int i,j;
		for (i = wx-brushw/2.0; i < wx+brushw/2.0; i++)
			for (j = wy-brushh/2.0; j < wy+brushh/2.0; j++) 
				if (i >= 0 && i < fft->ts && j >= 0 && j < fft->fs)
					fft->amp[i][j] = min;
	}
	running = False;
}

/* HELP IS A WORK IN PROGRESS */
void xcairo_help() {
	Window hwin = XCreateSimpleWindow(dpy,win,80,80,sw-160,sh-160,0,2,0);
	XMapWindow(dpy,hwin);
	cairo_surface_t *t = cairo_xlib_surface_create(dpy,hwin,
			DefaultVisual(dpy,scr),sw-160,sh-160);
	cairo_t *h = cairo_create(t);
	cairo_surface_destroy(t);
	cairo_surface_t *img = cairo_image_surface_create_from_png(HELP_FILE);
	cairo_scale(h,(sw-160)/641.0,(sh-160)/481.0);
	cairo_set_source_surface(h,img,0,0);
	cairo_paint(h);
	XFlush(dpy);
	cairo_surface_destroy(img);
	cairo_destroy(h);
	XEvent ev;
	while (!XNextEvent(dpy,&ev)) if (ev.type == KeyPress) break;
	XDestroyWindow(dpy,hwin);
	XFlush(dpy);
}

void zoom() {
	XEvent e;
	zoomer = 1; draw();
	while (e.type != ButtonPress) {
		XMaskEvent(dpy,ButtonPressMask|PointerMotionMask|KeyPressMask,&e);
		if (e.type == KeyPress) {
			zoomer=0;
			return;
		}
		sx = e.xbutton.x; sy = e.xbutton.y;
		wx = (sx - bw)/scx;
		wy = ffth + ffty + (sy - bw)/scy;
		draw();
	}
	range[0] = wx;
	lopass = fft->freq[(wy > 0 ? (wy < fft->fs ? wy : fft->fs-1) : 0)];
	zpy = wy;
	zoomer = 2; draw();
	e.type = MotionNotify;
	while (e.type != ButtonPress) {
		XMaskEvent(dpy,ButtonPressMask|PointerMotionMask,&e);
		sx = e.xbutton.x; sy = e.xbutton.y;
		wx = (sx - bw)/scx;
		wy = ffth + ffty + (sy - bw)/scy;
		draw();
	}
	range[1] = wx;
	hipass = fft->freq[(wy > 0 ? (wy < fft->fs ? wy : fft->fs-1) : 0)];
	if (range[1] < range[0]) {
		int t = range[0];
		range[0] = range[1];
		range[1] = t;
	}
	if (lopass < hipass) {
		double t = lopass;
		lopass = hipass;
		hipass = lopass;
	}
	running = False;
	restart = 2;
	review = 0;
}

int preview_init() {
	dpy = XOpenDisplay(0x0);
	scr = DefaultScreen(dpy);
	root = RootWindow(dpy,scr);
	sw = DisplayWidth(dpy,scr);
	sh = DisplayHeight(dpy,scr);
	sx = sw/2; sy = sh/2;
	XGCValues val;
	val.font = XLoadFont(dpy,font);
	fontstruct = XQueryFont(dpy,val.font);
	gc = XCreateGC(dpy,root,GCFont,&val);
	ty = fontstruct->ascent + 2;
	bw = fontstruct->descent + 4 + ty;
	XSetWindowAttributes wa;
	wa.override_redirect = True;
	win = XCreateWindow(dpy,root,0,0,sw,sh,0,DefaultDepth(dpy,scr),
			CopyFromParent, DefaultVisual(dpy,scr),CWOverrideRedirect, &wa);
	char curs_data = 0;
	XColor col;
	Pixmap curs_map = XCreateBitmapFromData(dpy,win,&curs_data,1,1);
	invisible_cursor = XCreatePixmapCursor(dpy,curs_map,curs_map,&col, &col,0,0);
	crosshair_cursor = XCreateFontCursor(dpy,XC_crosshair);
	XDefineCursor(dpy,win,crosshair_cursor);
	buf = XCreatePixmap(dpy,win,sw,sh,DefaultDepth(dpy,scr));
	pbuf = XCreatePixmap(dpy,win,sw,sh,DefaultDepth(dpy,scr));
	XMapRaised(dpy,win);
	XSetForeground(dpy,gc,BlackPixel(dpy,scr));
	XFillRectangle(dpy,buf,gc,0,0,sw,sh);
	XSetForeground(dpy,gc,WhitePixel(dpy,scr));
	XSetBackground(dpy,gc,BlackPixel(dpy,scr));
	XFlush(dpy);
	XWarpPointer(dpy,None,win,0,0,0,0,sw/2,sh/2);
	return 0;
}

int preview_create(int w, int h, FFT *fftp) {
XSetForeground(dpy,gc,BlackPixel(dpy,scr));
XFillRectangle(dpy,buf,gc,0,0,sw,sh);
XSetForeground(dpy,gc,WhitePixel(dpy,scr));
	fft = fftp;
	fftw = w; ffth = h;
	int i,j;
	for (i = 0; i < fft->fs && fft->freq[i] < hipass; i++);
	ffty = i;
	for (i++; i < fft->fs && fft->freq[i] < lopass; i++);
	ffth = i - ffty;
	brushw = fftw/20; brushh = ffth/15;
	restart = 0; zoomer = 0; eraser = False;
//
	cairo_surface_t *t = cairo_xlib_surface_create(dpy,buf,
			DefaultVisual(dpy,scr),sw,sh);
	c = cairo_create(t);
	l = cairo_create(t);
	//z = cairo_create(t);
	cairo_translate(c,bw,bw);
	cairo_translate(l,bw,bw);
	scx = (float)(sw-2*bw)/(float)fftw;
	scy = -1.0 *(float)(sh-2*bw)/(float)ffth;
	wx = (sw/2 - bw)/scx;
	wy = fft->fs + (sh/2 - bw)/scy;
	cairo_scale(c,scx,scy);
	cairo_translate(c,0.0,-1.0*ffth);
	cairo_scale(l,scx,scy);
	cairo_translate(l,0.0,-1.0*ffth);
	cairo_set_line_join(c,CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_join(l,CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c,0.5);
	cairo_set_line_width(l,line_width);
	cairo_surface_destroy(t);
	stride = cairo_format_stride_for_width(CAIRO_FORMAT_A8,fftw);
	alphas = (unsigned char *) malloc(stride * ffth * 
			sizeof(unsigned char));
	unsigned char *a = NULL;
//unsigned long ta = 0;
	for (j = 0; j < ffth; j++) {
		a = alphas + j*stride;
		for (i = 0; i < w; i++, a++) {
			*a = (unsigned char) 255 * (
					((fft->amp[i][j+ffty] - sp_floor) >= 0) 
					? fft->amp[i][j+ffty]/sp_floor : 1);
if (j == 0) {
fprintf(stderr,"%d: %lu\n",i,*a);
}
//ta += *a;
		}
//fprintf(stderr,"%d: %lu\n",j,ta);
//ta = 0;
	}
	mask = cairo_image_surface_create_for_data(alphas,CAIRO_FORMAT_A8,
			fftw,ffth,stride);
	cairo_set_source_rgba(c,1.0,1.0,1.0,1.0);
	cairo_mask_surface(c,mask,0,0);
	return 0;
}

int preview_threshold_start() {
	cairo_set_source_rgba(c,0.0,0.75,1.0,0.5);
}

int preview_threshold(int x, int y) {
	cairo_new_sub_path(c);
	cairo_arc(c,x,y-ffty,radius,0,2*M_PI);
	return 0;
}

int preview_peak_start() {
	cairo_fill(c);
	cairo_set_source_rgba(c,1.0,0.2,0.0,1.0);
	cairo_set_source_rgba(l,1.0,0.9,0.0,1.0);
}

int preview_peak(int x, int y) {
	cairo_new_sub_path(c);
	cairo_arc(c,x,y-ffty,radius,0,2*M_PI);
	cairo_line_to(l,x,y-ffty);
	return 0;
}

int preview_test(long double ee, long double te) {
	ex = ee; tex = te;
	cairo_set_source_rgba(c,1.0,0.2,0.0,1.0);
	cairo_set_source_rgba(l,1.0,0.9,0.0,1.0);
	cairo_stroke(l);
	cairo_fill(c);
	cairo_set_source_rgba(c,0.6,0.7,1.0,0.65);
	XCopyArea(dpy,buf,pbuf,gc,0,0,sw,sh,0,0);
	draw();
	XEvent ev;
	XRaiseWindow(dpy,win);
	int i;
	for (i = 0; i < 1000; i++) {
		if (XGrabKeyboard(dpy,root,True,GrabModeAsync,GrabModeAsync,CurrentTime) ==
				GrabSuccess) break;
		usleep(1000);
	}
	XGrabPointer(dpy, win, True, PointerMotionMask | ButtonPressMask |
			ButtonReleaseMask,GrabModeAsync,GrabModeAsync,win,None,CurrentTime);
	running = True; review = 1;
	XSync(dpy,True);
	while (running && !XNextEvent(dpy,&ev))
		if (ev.type < 33 && handler[ev.type]) handler[ev.type](&ev);
	XSetForeground(dpy,gc,BlackPixel(dpy,scr));
	XFillRectangle(dpy,buf,gc,0,0,sw,sh);
	XSetForeground(dpy,gc,WhitePixel(dpy,scr));
	cairo_set_source_rgba(c,1.0,1.0,1.0,1.0);
	cairo_mask_surface(c,mask,0,0);
	cairo_set_source_rgba(c,1.0,0.9,0.0,1.0);
	XUngrabPointer(dpy,CurrentTime);
	XUngrabKeyboard(dpy,CurrentTime);
	return review;
}

int preview_destroy() {
	cairo_surface_destroy(mask);
	cairo_surface_destroy(zsrc);
	cairo_destroy(c);
	cairo_destroy(l);
	free(alphas);
	return restart;
}

int preview_end() {
	XFreeFont(dpy,fontstruct);
	XFreeGC(dpy,gc);
	XFreePixmap(dpy,pbuf);
	XFreePixmap(dpy,buf);
	XDestroyWindow(dpy,win);
	XCloseDisplay(dpy);
	return 0;
}

