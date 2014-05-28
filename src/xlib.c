/**********************************************************************\
* FEX - The Frequency Excursion Calculator
*
* Author: Jesse McClure, copyright 2013-2014
* License: GPL3
*
*    This program is free software: you can redistribute it and/or
*    modify it under the terms of the GNU General Public License as
*    published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see
*    <http://www.gnu.org/licenses/>.
*
\**********************************************************************/

#include "fex.h"

#define MODE_NULL		0x00
#define MODE_ERASE	0x01
#define MODE_CROP		0x02

#define EVENT_MASK	ExposureMask | ButtonPressMask | KeyPressMask | \
							StructureNotifyMask

static void buttonpress(XEvent *);
static void clientmessage(XEvent *);
static void configurenotify(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void motionnotify(XEvent *);

static int crop(int, int);
static int erase(int, int);
static int eraser_cursor(int, int);
static int move(double, double);
static int play(const char *);
static int pt_line(double, double);
static int screenshot();
static int sp_floor(double);
static int threshold(double);
static int zoom(double);

static Display *dpy;
static int scr, ww, wh, mx, my;
static GC gc;
static Window root, win;
static Pixmap buf, ibuf, sbuf;
static Atom WM_DELETE_WINDOW;
static cairo_t *ictx, *sctx;
static Bool running = True;
static double xsc = 1.0, ysc = 1.0, xoff = 0.0, yoff = 0.0;
static int mode = 0, ew = 31, eh = 64;
static char e_col[2][8];
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]			= buttonpress,
	[ClientMessage]		= clientmessage,
	[ConfigureNotify]		= configurenotify,
	[Expose]					= expose,
	[KeyPress]				= keypress,
	[MotionNotify]			= motionnotify,
};

#include "xlib_toolwin.c"

#define CtrlMask	ControlMask
void buttonpress(XEvent *ev) {
	XButtonEvent *e = &ev->xbutton;
	if (e->window == info->win) info->button(info, e);
	else if (e->state == (ControlMask | ShiftMask)) {
		if (e->button == 4) threshold(0.1);
		else if (e->button == 5) threshold(-0.1);
		else if (e->button == 6) sp_floor(-0.1);
		else if (e->button == 7) sp_floor(0.1);
	}
	else if (e->state == ControlMask) {
		if (e->button == 4) zoom(0.025);
		else if (e->button == 5) zoom(-0.025);
		else if (e->button == 6) return;
		else if (e->button == 7) return;
	}
	else if (e->state == Mod1Mask) {
		if (e->button == 4) eraser_cursor(1,1);
		else if (e->button == 5) eraser_cursor(-1,-1);
		else if (e->button == 6) eraser_cursor(-1,1);
		else if (e->button == 7) eraser_cursor(1,-1);
	}
	else if (e->state == ShiftMask) {
		if (e->button == 4) pt_line(0.2,0);
		else if (e->button == 5) pt_line(-0.2,0);
		else if (e->button == 6) pt_line(0,-0.2);
		else if (e->button == 7) pt_line(0,0.2);
	}
	else if (e->button == 4) move(0,-0.02);
	else if (e->button == 5) move(0,0.02);
	else if (e->button == 6) move(0.02,0);
	else if (e->button == 7) move(-0.02,0);
	else if (mode == MODE_ERASE && e->button == 1) erase(e->x, e->y);
	else if (mode == MODE_ERASE && e->button == 3) erase(-1,-1);
	else if (mode == MODE_CROP && e->button == 1) crop(e->x, e->y);
	while(XCheckMaskEvent(dpy, ButtonPressMask, ev));
}

void clientmessage(XEvent *ev) {
	XClientMessageEvent *e = &ev->xclient;
	if ( (Atom)e->data.l[0] == WM_DELETE_WINDOW ) {
		if (e->window == info->win) {
			info->vis = False;
			XUnmapWindow(dpy, info->win);
		}
		else if (e->window == win) {
			running = False;
		}
	}
}

void configurenotify(XEvent *ev) {
	while (XCheckTypedEvent(dpy,ConfigureNotify, ev));
	XConfigureEvent *e = &ev->xconfigure;
	if (e->window == win) {
		ww = e->width; wh = e->height;
		spectro_draw();
		XSetWindowBackgroundPixmap(dpy, win, buf);
		XClearWindow(dpy,win);
	}
}

void expose(XEvent *ev) {
	XExposeEvent *e = &ev->xexpose;
	if (e->window == info->win) info->draw(info);
	else {
		XSetWindowBackgroundPixmap(dpy, win, buf);
		XClearWindow(dpy,win);
	}
}

void keypress(XEvent *ev) {
	XKeyEvent *e = &ev->xkey;
	KeySym sym = XkbKeycodeToKeysym(dpy, (KeyCode)e->keycode, 0, 0);
	int mod = ((e->state & ~Mod2Mask) & ~LockMask);
	if (mod == (ControlMask | ShiftMask)) {
		if (sym == XK_q) {
			spect->fex = 0;
			running = False;
		}
		if (sym == XK_j || sym == XK_Down) threshold(-0.05);
		else if (sym == XK_k || sym == XK_Up) threshold(0.05);
		else if (sym == XK_h || sym == XK_Left) sp_floor(-0.05);
		else if (sym == XK_l || sym == XK_Right) sp_floor(0.05);
	}
	else if (mod == ControlMask) {
		if (sym == XK_q) running = False;
		if (sym == XK_s) screenshot();
		else if (sym == XK_j || sym == XK_Down) zoom(-0.025);
		else if (sym == XK_k || sym == XK_Up) zoom(0.025);
		else if (sym == XK_h || sym == XK_Left) return;
		else if (sym == XK_l || sym == XK_Right) return;
	}
	else if (mod == Mod1Mask) {
		if (sym == XK_j || sym == XK_Down) eraser_cursor(-1,-1);
		else if (sym == XK_k || sym == XK_Up) eraser_cursor(1,1);
		else if (sym == XK_h || sym == XK_Left) eraser_cursor(-1,1);
		else if (sym == XK_l || sym == XK_Right) eraser_cursor(1,-1);
	}
	else if (mod == ShiftMask) {
		if (sym == XK_j || sym == XK_Down) pt_line(-0.2,0);
		else if (sym == XK_k || sym == XK_Up) pt_line(0.2,0);
		else if (sym == XK_h || sym == XK_Left) pt_line(0,-0.2);
		else if (sym == XK_l || sym == XK_Right) pt_line(0,0.2);
		else if (sym == XK_p) play("0.5");
	}
	else if (sym == XK_j || sym == XK_Down) move(0,0.02);
	else if (sym == XK_k || sym == XK_Up) move(0,-0.02);
	else if (sym == XK_h || sym == XK_Left) move(0.02,0);
	else if (sym == XK_l || sym == XK_Right) move(-0.02,0);
	else if (sym == XK_F1) {
		if (fork() == 0)
			execvp(conf.help_cmd[0],(char * const *)conf.help_cmd);
	}
	else if (sym == XK_F2) {
		if ( (info->vis = !info->vis) ) XMapRaised(dpy,info->win);
		else XUnmapWindow(dpy,info->win);
		XFlush(dpy);
	}
	else if (sym == XK_e) {
		mode = MODE_ERASE & (mode ^= MODE_ERASE);
		eraser_cursor(0,0);
		info->draw(info);
	}
	else if (sym == XK_c) {
		mode = MODE_CROP & (mode ^= MODE_CROP);
		if (!(mode & MODE_CROP)) XDefineCursor(dpy, win, None);
		else XDefineCursor(dpy, win, XCreateFontCursor(dpy, 34));
		info->draw(info);
	}
	else if (sym == XK_Escape) {
		mode = MODE_NULL;
		XDefineCursor(dpy, win, None);
		info->draw(info);
	}
	else if (sym == XK_p) play("1.0");
	else if (sym == XK_l) {
		conf.layers = !conf.layers;
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	}
	else if (sym == XK_u && mode & (MODE_ERASE)) erase(-1,-1);
	while(XCheckMaskEvent(dpy, KeyPressMask, ev));
}

void motionnotify(XEvent *ev) {
	static int px, py;
	int x = ev->xmotion.x, y = ev->xmotion.y;
	mx = spect->fft_w * x / (xsc * ww) - 1.0 * xoff + spect->fft_x;
	my = spect->fft_h * (1.0 - y / (ysc * wh)) - 1.0 * yoff + spect->fft_y;
	info->draw(info);
	if (mode == MODE_CROP) {
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		XDrawLine(dpy, win, gc, x, 0, x, wh);
		XDrawLine(dpy, win, gc, 0, y, ww, y);
	}
}


int crop(int x, int y) {
	int x1 = x, y1 = y, x2, y2, t;
	XCopyArea(dpy, win, buf, gc, 0, 0, ww, wh, 0, 0);
	XEvent e;
	XGrabPointer(dpy, win, True, PointerMotionMask | ButtonPressMask,
			GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	while (e.type != ButtonPress) {
		XMaskEvent(dpy, PointerMotionMask | ButtonPressMask |
				KeyPressMask, &e);
		if (e.type == KeyPress) break;
		x2 = e.xmotion.x;
		y2 = e.xmotion.y;
		if (x2 > ww) x2 = ww;
		if (y2 > wh) y2 = wh;
		mx = spect->fft_w * x2/(ww*xsc) - 1.0 * xoff + spect->fft_x;
		my = spect->fft_h * (1.0-y2/(wh*ysc)) - 1.0 * yoff + spect->fft_y;
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		XDrawLine(dpy, win, gc, x2, 0, x2, wh);
		XDrawLine(dpy, win, gc, 0, y2, ww, y2);
		info_draw(info);
		if (e.type == ButtonPress) break;
		while(XCheckMaskEvent(dpy, PointerMotionMask, &e));
	}
	XUngrabPointer(dpy, CurrentTime);
	if (x2 < x1) { t = x1; x1 = x2; x2 = t; }
	if (y2 > y1) { t = y1; y1 = y2; y2 = t; }
	mode = MODE_NULL;
	info_draw(info);
	if (e.type == KeyPress) {
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		return 0;
	}
	int oldy = spect->fft_y;
	spect->fft_x = spect->fft_w * x1/(ww*xsc) - 1.0*xoff + spect->fft_x;
	spect->fft_y = spect->fft_h*(1.0-y1/(wh*ysc))-1.0*yoff+spect->fft_y;
	spect->fft_w = spect->fft_w * x2/(ww*xsc) - 1.0*xoff - spect->fft_x;
	spect->fft_h = spect->fft_h*(1.0-y2/(wh*ysc))-1.0*yoff -
		oldy + spect->fft_x;
	spectro_spec();
	spectro_thresh();
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	return 0;
}

int erase(int x, int y) {
	int x1, x2, y1, y2, i, j;
	double sew = spect->fft_w * ew / (ww * xsc);
	double seh = spect->fft_h * eh / (wh * ysc);
	if (x == -1 && y == -1) { /* UNDO LAST ERASE */
		for (i = 0; i < spect->fft->ntime; i++)
			for (j = 0; j < spect->fft->nfreq; j++)
				spect->fft->mask[i][j] = ( (spect->fft->mask[i][j] >> 7) ?
						(spect->fft->mask[i][j] >> 1) | 0x40 :
						(spect->fft->mask[i][j] >> 1) );
		spectro_points();
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		return 0;
	} /* ELSE START NEW ERASE */
	XEvent e;
	XGrabPointer(dpy, win, True, PointerMotionMask | ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	/* rotate bits for 7 undo levels */
	for (i = 0; i < spect->fft->ntime; i++)
		for (j = 0; j < spect->fft->nfreq; j++)
			spect->fft->mask[i][j] = ( (spect->fft->mask[i][j] >> 7) ?
					(spect->fft->mask[i][j] << 1) | 0x40 :
					(spect->fft->mask[i][j] << 1) );
	while (e.type != ButtonRelease) {
		XMaskEvent(dpy, PointerMotionMask | ButtonReleaseMask, &e);
		/* set erase bit mask */
		mx = spect->fft_w*e.xbutton.x/(ww*xsc) - 1.0*xoff + spect->fft_x;
		my = spect->fft_h*(1.0-e.xbutton.y/(wh*ysc)) -1.0*yoff+spect->fft_y;
		x1 = mx - sew / 2.0; y1 = my - seh / 2.0;
		x2 = mx + sew / 2.0; y2 = my + seh / 2.0;
		if (x1 < 0) x1 = 0;
		if (y1 < 0) y1 = 0;
		if (x2 >= spect->fft->ntime) x2 = spect->fft->ntime - 1;
		if (y2 >= spect->fft->nfreq) y2 = spect->fft->nfreq - 1;
		for (j = y1; j <= y2; j++)
			for (i = x1; i <= x2; i++)
				spect->fft->mask[i][j] |= 0x01;
		/* redraw */
		spectro_thresh();
		spectro_points();
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		info_draw(info);
		if (e.type == ButtonRelease) break;
		while(XCheckMaskEvent(dpy, PointerMotionMask, &e));
	}
	XUngrabPointer(dpy, CurrentTime);
}

int eraser_cursor(int w, int h) {
	if ( !(mode & MODE_ERASE) ) {
		XDefineCursor(dpy, win, None);
		return 0;
	}
	if ( (ew+=w) < 3 ) ew = 3;
	if ( (eh+=h) < 3 ) eh = 3;
	if ( ew > ww/4 ) ew = ww / 4;
	if ( eh > wh/2 ) eh = wh / 2;
	int i, j, stride = ew/ 8 + 1;
	char data[stride * eh];
	char mask[stride * eh];
	XColor bg, fg;
	XAllocNamedColor(dpy,DefaultColormap(dpy,scr),e_col[0],&bg,&bg);
	XAllocNamedColor(dpy,DefaultColormap(dpy,scr),e_col[1],&fg,&fg);
	memset(mask, 0xAA, stride * eh);
	memset(data, 0xAA, stride * eh);
	for (i = 0; i < eh; i+= 2) memset(&mask[stride * i], 0x55, stride);
	Pixmap cd = XCreateBitmapFromData(dpy, win, data, ew, eh);
	Pixmap cm = XCreateBitmapFromData(dpy, win, mask, ew, eh);
	Cursor c = XCreatePixmapCursor(dpy, cd, cm, &fg, &bg, ew/2, eh/2);
	XDefineCursor(dpy, win, c);
	XFreePixmap(dpy, cd);
	XFreePixmap(dpy, cm);
	XFlush(dpy);
}

int move(double x, double y) {
	xoff += x / xsc * spect->fft_w; yoff += y / ysc * spect->fft_h;
	if ( yoff > spect->fft_h * (1.0 - 1.0/ysc) )
		yoff = spect->fft_h * (1.0 - 1.0/ysc);
	if ( yoff < 0) yoff = 0;
	if ( xoff < spect->fft_w * (1.0/xsc - 1.0) )
		xoff = spect->fft_w * (1.0/xsc - 1.0);
	if ( xoff > 0) xoff = 0;
	if (x || y) { /* don't redraw for offset checks */
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	}
}

int play(const char *speed) {
	if (!fork()==0) {
		close(ConnectionNumber(dpy));
		char start[12], end[12], mid[12], wid[12];
		double _start = spect->fft->time[spect->fft_x];
		double _end = spect->fft->time[spect->fft_x+spect->fft_w-1];
		double _low = spect->fft->freq[spect->fft_y];
		double _hi = spect->fft->freq[spect->fft_y+spect->fft_h-1];
		double _mid = (_hi+_low)/2.0;
		double _wid = (_hi-_low)/2.0;
		sprintf(start,"=%lf",_start); sprintf(end,"=%lf",_end);
		sprintf(mid,"%lfk",_mid); sprintf(wid,"%lfk",_wid);
		execl("/usr/bin/play", "play", "-q", spect->fname,
				"trim", start, end, "bandpass", mid, wid,
				"speed", speed, NULL);
		perror("Fex Play");
		exit(1);
	}
}

int pt_line(double p, double l) {
	conf.col[RGBA_POINTS].w += p;
	conf.col[RGBA_LINES].w += l;
	if (conf.col[RGBA_POINTS].w < 0) conf.col[RGBA_POINTS].w = 0.0;
	if (conf.col[RGBA_LINES].w < 0) conf.col[RGBA_LINES].w = 0.0;
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
}

int screenshot() {
	char fname[256];
	static int n = 0;
	snprintf(fname, 256, "%s-%d.png", spect->name, n++);
	cairo_surface_t *t = cairo_xlib_surface_create(dpy, buf,
			DefaultVisual(dpy,scr), ww, wh);
	cairo_surface_write_to_png(t, fname);
	cairo_surface_destroy(t);
}

int sp_floor(double f) {
	conf.spect_floor += f;
	spectro_spec();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	if (info->vis) info->draw(info);
}

int threshold(double f) {
	conf.thresh += f;
	spectro_thresh();
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	if (info->vis) info->draw(info);
}

int zoom(double f) {
	double px = xsc, py = ysc;
	xsc += f; ysc += f;
	if (xsc < 1.0) xsc = 1.0;
	if (ysc < 1.0) ysc = 1.0;
	xoff += (spect->fft_w/xsc - spect->fft_w/px) / 2.0;
	yoff += (spect->fft_h/py - spect->fft_h/ysc) / 2.0;
	if (f < 0) move(0,0); /* check offsets for bounds */
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
}

int create_xlib() {
	if (!setlocale(LC_CTYPE,"")) die("unable to set locale");
	if (!XSupportsLocale()) die("unsupported locale");
	if (!XSetLocaleModifiers("")) die("unable to set locale modifiers");
	if ( !(dpy=XOpenDisplay(0x0)) ) die("unable to open display");
	scr = DefaultScreen(dpy);
	root = DefaultRootWindow(dpy);
	gc = DefaultGC(dpy,scr);
	ww = DisplayWidth(dpy,scr);
	wh = DisplayHeight(dpy,scr);
	/* create windows */
	buf = XCreatePixmap(dpy, root, ww, wh, DefaultDepth(dpy,scr));
	win = XCreateSimpleWindow(dpy,root,0,0,(ww*=0.85),(wh*=0.85),0,0, 0);
	XSelectInput(dpy, win, EVENT_MASK | SubstructureRedirectMask |
			PointerMotionMask);
	XStoreName(dpy, win, "FEX: Frequency Excursion Calculator");
	XClassHint *hint = XAllocClassHint();
	hint->res_name = "FEX";
	hint->res_class = "FEX";
	XSetClassHint(dpy, win, hint);
	WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);
#include "icon.xpm"
Pixmap image, shape;
XWMHints *hints;
XpmCreatePixmapFromData(dpy, win, icon, &image, &shape, NULL);
hints = XAllocWMHints();
hints->flags = IconPixmapHint | IconMaskHint;
hints->icon_pixmap = image;
hints->icon_mask = shape;
XSetWMHints(dpy, win, hints);
XFree(hints);
	/* set up eraser / crop colors */
	sprintf(e_col[0],"#%02hhX%02hhX%02hhX",
		(unsigned char) conf.col[RGBA_ERASE1].r * 255,
		(unsigned char) conf.col[RGBA_ERASE1].g * 255,
		(unsigned char) conf.col[RGBA_ERASE1].b * 255);
	sprintf(e_col[1],"#%02hhX%02hhX%02hhX",
		(unsigned char) conf.col[RGBA_ERASE2].r * 255,
		(unsigned char) conf.col[RGBA_ERASE2].g * 255,
		(unsigned char) conf.col[RGBA_ERASE2].b * 255);
	char c_col[8];
	XColor c;
	sprintf(c_col,"#%02hhX%02hhX%02hhX",
		(unsigned char) conf.col[RGBA_CROP].r * 255,
		(unsigned char) conf.col[RGBA_CROP].g * 255,
		(unsigned char) conf.col[RGBA_CROP].b * 255);
	XAllocNamedColor(dpy,DefaultColormap(dpy,scr),c_col,&c,&c);
	XSetLineAttributes(dpy, gc, conf.col[RGBA_CROP].w, LineSolid,
			CapButt, JoinRound);
	XSetForeground(dpy, gc, c.pixel);
	toolwin_create();
	spectro_draw();
	XSetWindowBackgroundPixmap(dpy, win, buf);
	return 0;
}

int free_xlib() {
	toolwin_destroy();
	XFlush(dpy);
	XCloseDisplay(dpy);
}

cairo_t *xlib_context() {
	cairo_surface_t *t = cairo_xlib_surface_create(dpy, buf,
			DefaultVisual(dpy,scr), ww, wh);
	cairo_t *c = cairo_create(t);
	cairo_surface_destroy(t);
	cairo_translate(c,0,wh * ysc);
	cairo_scale(c, (ww * xsc)/ (float) spect->fft_w,
			-1.0 * (wh * ysc) / (float) spect->fft_h);
	cairo_translate(c, xoff, yoff);
	return c;
}

int xlib_event_loop() {
	XMapWindow(dpy, win);
	XMapWindow(dpy, info->win);
	XFlush(dpy);
	XEvent ev;
	while (running && !XNextEvent(dpy,&ev))
		if (ev.type < LASTEvent && handler[ev.type])
			handler[ev.type](&ev);
}


