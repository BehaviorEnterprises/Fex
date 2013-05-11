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

static const char *font = "-*-terminus-bold-*-*-*-14-*-*-*-*-*-*-*";

static void buttonpress(XEvent *);
static void buttonrelease(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void motionnotify(XEvent *);
static void xcairo_help();

static Display *dpy;
static int scr, sw, sh, sx, sy, fftw, ffth, stride;
static int review, restart, ty, bw, wx, wy;
static Window root, win;
static Pixmap buf, pbuf, zmap;
static GC gc;
static XFontStruct *fontstruct;
static Cursor invisible_cursor, crosshair_cursor;
static Bool running, zoom, eraser;
static cairo_t *c, *z;
static cairo_surface_t *mask, *zsrc;
static float scx, scy, brushw, brushh;
static unsigned char *alphas, *a;
static FFT *fft;
static long double ex,tex;
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]	= buttonpress,
	[ButtonRelease]	= buttonrelease,
	[Expose]		= expose,
	[KeyPress]		= keypress,
	[MotionNotify]	= motionnotify,
};

void xcairo_help(){
	cairo_surface_t *t = cairo_xlib_surface_create(dpy,buf,
			DefaultVisual(dpy,scr),sw,sh);
	cairo_t *h = cairo_create(t);
	cairo_surface_destroy(t);
	double x = (sw-6*bw)/1600.0, y = (sh-6*bw)/750.0;
	cairo_translate(h,3*bw,3*bw);
	cairo_scale(h,x,y);
	cairo_set_line_join(h,CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(h,12.0);
	cairo_surface_t *src = cairo_image_surface_create_from_png("help.png");
	cairo_set_source_surface(h,src,0,0);
	cairo_paint(h);
	cairo_surface_destroy(src);
	cairo_set_source_rgba(h,0.0,0.1,0.2,0.8);
	cairo_rectangle(h,0,0,1600,750);
	cairo_stroke(h);
	cairo_destroy(h);
	XCopyArea(dpy,buf,win,gc,0,0,sw,sh,0,0);
	XFlush(dpy);
	XEvent e;
	XMaskEvent(dpy,KeyPressMask,&e);
	XSync(dpy,True);
}

void buttonpress(XEvent *e) {
	XSync(dpy,True);
}

void buttonrelease(XEvent *e) {
	XSync(dpy,True);
	running = False; /* trigger recalculation of peaks */
}

void draw() {
	XCopyArea(dpy,pbuf,buf,gc,0,0,sw,sh,0,0);
	if (zoom) XCopyArea(dpy,buf,zmap,gc, sx - sw/24, sy - sh/24,
			(sw-2*bw)/12+2,(sh-2*bw)/12+2,0,0);
	if (eraser) {
		cairo_rectangle(c,wx-brushw/2,wy-brushh/2,brushw,brushh);
		cairo_fill(c);
	}
	char str[256]; int x = bw + 4;
	sprintf(str,"time: %.3lfs | freq: %.3lfkhz",fft->time[wx],fft->freq[wy]);
	XDrawString(dpy,buf,gc,x,ty,str,strlen(str));
	sprintf(str,"path: %.2Lf | time: %.2Lf | FE: %.2Lf",ex,tex,ex/tex);
	x = XTextWidth(fontstruct,str,strlen(str)) + 4;
	XDrawString(dpy,buf,gc,sw - bw - x,ty,str,strlen(str));
	x = XTextWidth(fontstruct,name,strlen(name));
	x = (sw - x)/2;
	XDrawString(dpy,buf,gc,x,ty,name,strlen(name));
	if (zoom) {
		cairo_set_source_surface(z,zsrc,0,0);
		cairo_paint(z);
		cairo_set_source_rgba(z,0.0,0.2,0.4,0.8);
		cairo_rectangle(z,0,0,(sw-2*bw)/12,(sh-2*bw)/12);
		cairo_stroke(z);
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
	if (mod == ControlMask) {
		if(key == XK_q) { review = 0; running = False; }
		else if (key == XK_r) { restart = 1; review = 0; running = False; }
	}
	else if (mod == ShiftMask) {
		if (key == XK_Up) { brushw *= 1.2; brushh *=1.2; }
		else if (key == XK_Down) { brushw *= 1/1.2; brushh *=1/1.2; }
		else if (key == XK_Right) { brushw *= 1.2; brushh *=1/1.2; }
		else if (key == XK_Left) { brushw *= 1/1.2; brushh *=1.2; }
	}
	else if (key == XK_Up) { thresh *= 1.2; running = False; }
	else if (key == XK_Down) { thresh *= 1/1.2; running = False; }
	else if (key == XK_z) zoom = !zoom;
	else if (key == XK_h) xcairo_help();
	else if (key == XK_e) {
		if ( (eraser = !eraser) ) XDefineCursor(dpy,win,invisible_cursor);
		else XDefineCursor(dpy,win,crosshair_cursor);
	}
	draw();
}

void motionnotify(XEvent *e){
	sx = e->xbutton.x; sy = e->xbutton.y;
	wx = (sx - bw)/scx;
	wy = fft->fs + (sy - bw)/scy;
	if (e->xbutton.x > bw && e->xbutton.x < sw-bw &&
			e->xbutton.y > bw && e->xbutton.y < sh-bw &&
			((e->xbutton.state & Button1Mask) == Button1Mask)) {
		int i,j;
		for (i = wx-brushw/2.0; i < wx+brushw/2.0; i++)
			for (j = wy-brushh/2.0; j < wy+brushh/2.0; j++) 
				if (i >= 0 && i < fft->ts && j >= 0 && j < fft->fs)
					fft->amp[i][j] = min;
	}
	draw();
}

int preview_create(int w, int h, FFT *fftp) {
	fft = fftp;
	fftw = w; ffth = h;
	brushw = w/14; brushh = h/14;
	restart = 0; zoom = eraser = False;
	int i,j;
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
	zmap = XCreatePixmap(dpy,win,sw/10,sh/10,DefaultDepth(dpy,scr));
	XMapWindow(dpy,win);
	XSetForeground(dpy,gc,BlackPixel(dpy,scr));
	XFillRectangle(dpy,buf,gc,0,0,sw,sh);
	XSetForeground(dpy,gc,WhitePixel(dpy,scr));
	XSetBackground(dpy,gc,BlackPixel(dpy,scr));
	XFlush(dpy);
	XWarpPointer(dpy,None,win,0,0,0,0,sw/2,sh/2);
	wx = (sw/2 - bw)/scx;
	wy = fft->fs + (sh/2 - bw)/scy;
	cairo_surface_t *t = cairo_xlib_surface_create(dpy,buf,
			DefaultVisual(dpy,scr),sw,sh);
	c = cairo_create(t);
	z = cairo_create(t);
	cairo_translate(c,bw,bw);
	scx = (float)(sw-2*bw)/(float)w;
	scy = -1.0 *(float)(sh-2*bw)/(float)h;
	cairo_scale(c,scx,scy);
	zsrc = cairo_xlib_surface_create(dpy,zmap,
			DefaultVisual(dpy,scr),(sw-2*bw)/12,(sh-2*bw)/12);
	cairo_translate(z,bw*2,bw*2);
	cairo_scale(z,4.0,4.0);
	cairo_translate(c,0.0,-1.0 * h);
	cairo_set_line_join(z,CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(z,3.0);
	cairo_set_line_join(c,CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c,0.1);
	cairo_surface_destroy(t);
	stride = cairo_format_stride_for_width(CAIRO_FORMAT_A8,w);
	alphas = malloc(stride * h);
	for (j = 0; j < h; j++) {
		a = alphas + stride*j;
		for (i = 0; i < w; i++, a++)
			*a = (unsigned char) (fft->amp[i][j] * 255/ min);
	}
	mask = cairo_image_surface_create_for_data(alphas,CAIRO_FORMAT_A8,w,h,stride);
	cairo_set_source_rgba(c,1.0,1.0,1.0,1.0);
	cairo_mask_surface(c,mask,0,0);
	cairo_set_source_rgba(c,1.0,0.9,0.0,1.0);
	return 0;
}

int preview_peak(int x, int y) {
	cairo_new_sub_path(c);
	cairo_arc(c,x,y,0.35,0,2*M_PI);
	return 0;
}

int preview_test(long double ee, long double te) {
	ex = ee; tex = te;
	cairo_fill(c);
	cairo_set_source_rgba(c,0.6,0.7,1.0,0.65);
	XCopyArea(dpy,buf,pbuf,gc,0,0,sw,sh,0,0);
	draw();
	XEvent ev;
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
	cairo_destroy(z);
	free(alphas);
	XFreeFont(dpy,fontstruct);
	XFreeGC(dpy,gc);
	XFreePixmap(dpy,pbuf);
	XFreePixmap(dpy,buf);
	XDestroyWindow(dpy,win);
	XCloseDisplay(dpy);
	return restart;
}

