
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

static int erase(int, int);
static int eraser_cursor(int, int);
static int move(double, double);
static int pt_line(double, double);
static int sp_floor(double);
static int threshold(double);
static int zoom(double);

static Display *dpy;
static int scr, ww, wh;
static GC gc;
static Window root, win;
static Pixmap buf, ibuf, sbuf;
static Atom WM_DELETE_WINDOW;
static cairo_t *ictx, *sctx;
static Bool running = True;
static double xsc = 1.0, ysc = 1.0, xoff = 0.0, yoff = 0.0;
static int mode = 0, ew = 32, eh = 64;
static char e_col[2][8];
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]			= buttonpress,
	[ClientMessage]		= clientmessage,
	[ConfigureNotify]		= configurenotify,
	[Expose]					= expose,
	[KeyPress]				= keypress,
};

#include "xlib_toolwin.c"

#define CtrlMask	ControlMask
void buttonpress(XEvent *ev) {
	XButtonEvent *e = &ev->xbutton;
	if (e->window == info->win) {
		if (info->button) info->button(info);
		return;
	}
	else if (e->window == help->win) {
		if (help->button) help->button(help);
		return;
	}
	else if (e->state == (ControlMask | ShiftMask)) {
		if (e->button == 4) threshold(0.1);
		else if (e->button == 5) threshold(-0.1);
		else if (e->button == 6) sp_floor(-0.1);
		else if (e->button == 7) sp_floor(0.1);
	}
	else if (e->state == ControlMask) {
		if (e->button == 3) running = False;
		else if (e->button == 4) zoom(0.025);
		else if (e->button == 5) zoom(-0.025);
		else if (e->button == 6) return;
		else if (e->button == 7) return;
	}
	else if (e->state == Mod1Mask) {
		if (e->button == 4) eraser_cursor(1,1);
		else if (e->button == 5) eraser_cursor(-1,-1);
		else if (e->button == 6) eraser_cursor(1,-1);
		else if (e->button == 7) eraser_cursor(-1,1);
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
	else if (mode == MODE_ERASE && e->button == 2) erase(-1,-1);
	XSync(dpy,True);
}

void clientmessage(XEvent *ev) {
	XClientMessageEvent *e = &ev->xclient;
	if ( (Atom)e->data.l[0] == WM_DELETE_WINDOW ) {
		if (e->window == info->win) {
			info->vis = False;
			XUnmapWindow(dpy, info->win);
		}
		else if (e->window == help->win) {
			help->vis = False;
			XUnmapWindow(dpy, help->win);
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
	}
}

void expose(XEvent *ev) {
	XExposeEvent *e = &ev->xexpose;
	if (e->window == info->win) {
		if (info->draw) info->draw(info);
		else toolwin_draw(info);
	}
	else if (e->window == help->win) {
		if (help->draw) help->draw(help);
		else toolwin_draw(help);
	}
	else {
		XSetWindowBackgroundPixmap(dpy, win, buf);
		XClearWindow(dpy,win);
		//XFlush(dpy);
	}
}

void keypress(XEvent *ev) {
	XKeyEvent *e = &ev->xkey;
	KeySym sym = XkbKeycodeToKeysym(dpy, (KeyCode)e->keycode, 0, 0);
	int mod = ((e->state & ~Mod2Mask) & ~LockMask);
	if (sym == XK_F1) {
		if ( (help->vis = !help->vis) ) XMapRaised(dpy, help->win);
		else XUnmapWindow(dpy, help->win);
		XFlush(dpy);
	}
	else if (sym == XK_F2) {
		if ( (info->vis = !info->vis) ) XMapRaised(dpy,info->win);
		else XUnmapWindow(dpy,info->win);
		XFlush(dpy);
	}
	else if (sym == XK_e) {
		mode ^= MODE_ERASE;
		eraser_cursor(0,0);
	}
	else if (sym == XK_Escape) mode = MODE_NULL;
	else if (mod == ControlMask && sym == XK_j) zoom(0.025);
	else if (mod == ControlMask && sym == XK_k) zoom(-0.025);
	else if (mod == ControlMask && sym == XK_h) return;
	else if (mod == ControlMask && sym == XK_l) return;
	else if (mod == Mod1Mask && sym == XK_j) threshold(-0.1);
	else if (mod == Mod1Mask && sym == XK_k) threshold(0.1);
	else if (mod == Mod1Mask && sym == XK_h) sp_floor(-0.1);
	else if (mod == Mod1Mask && sym == XK_l) sp_floor(0.1);
	else if (mod == ShiftMask && sym == XK_j) pt_line(-0.2,0);
	else if (mod == ShiftMask && sym == XK_k) pt_line(0.2,0);
	else if (mod == ShiftMask && sym == XK_h) pt_line(0,-0.2);
	else if (mod == ShiftMask && sym == XK_l) pt_line(0,0.2);
	else if (sym == XK_j) move(0,0.02);
	else if (sym == XK_k) move(0,-0.02);
	else if (sym == XK_h) move(0.02,0);
	else if (sym == XK_l) move(-0.02,0);
	XSync(dpy,True);
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
		x1 = spect->fft_w * e.xbutton.x / (ww * xsc) - 1.0 * xoff;
		y1 = spect->fft_h * (1.0 - e.xbutton.y / (wh * ysc)) - 1.0 * yoff;
		x1 -= sew / 2.0; y1 -= seh / 2.0; x2 = x1 + sew; y2 = y1 + seh;
		if (x1 < 0) x1 = 0;
		if (y1 < 0) y1 = 0;
		if (x2 >= spect->fft->ntime) x2 = spect->fft->ntime - 1;
		if (y2 >= spect->fft->nfreq) y2 = spect->fft->nfreq - 1;
		for (j = y1; j <= y2; j++)
			for (i = x1; i <= x2; i++)
				spect->fft->mask[i+spect->fft_x][j+spect->fft_y] |= 0x01;
		/* redraw */
		spectro_thresh();
		spectro_points();
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		XCheckMaskEvent(dpy, ButtonReleaseMask, &e);
		XSync(dpy,True);
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
	if ( eh > wh/4 ) eh = wh / 4;
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

int pt_line(double p, double l) {
	conf.col[RGBA_POINTS].w += p;
	conf.col[RGBA_LINES].w += l;
	if (conf.col[RGBA_POINTS].w < 0) conf.col[RGBA_POINTS].w = 0.0;
	if (conf.col[RGBA_LINES].w < 0) conf.col[RGBA_LINES].w = 0.0;
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
}

int sp_floor(double f) {
	conf.spect_floor += f;
	spectro_spec();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
}

int threshold(double f) {
	conf.thresh += f;
	spectro_thresh();
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
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
//	if (!setlocale(LC_CTYPE,"")) die("unable to set locale");
//	if (!XSupportsLocale()) die("unsupported locale");
//	if (!XSetLocaleModifiers("")) die("unable to set locale modifiers");
	if ( !(dpy=XOpenDisplay(0x0)) ) die("unable to open display");
	scr = DefaultScreen(dpy);
	root = DefaultRootWindow(dpy);
	gc = DefaultGC(dpy,scr);
	ww = DisplayWidth(dpy,scr);
	wh = DisplayHeight(dpy,scr);
	/* create windows */
	buf = XCreatePixmap(dpy, root, ww, wh, DefaultDepth(dpy,scr));
	win = XCreateSimpleWindow(dpy,root,0,0,(ww*=0.85),(wh*=0.85),0,0, 0);
	XSelectInput(dpy, win, EVENT_MASK | SubstructureRedirectMask);
	XStoreName(dpy, win, "FEX: Frequency Excursion Calculator");
	WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);
	/* set eraser colors */
	sprintf(e_col[0],"#%02hhX%02hhX%02hhX",
		(unsigned char) conf.col[RGBA_ERASE1].r * 255,
		(unsigned char) conf.col[RGBA_ERASE1].g * 255,
		(unsigned char) conf.col[RGBA_ERASE1].b * 255);
	sprintf(e_col[1],"#%02hhX%02hhX%02hhX",
		(unsigned char) conf.col[RGBA_ERASE2].r * 255,
		(unsigned char) conf.col[RGBA_ERASE2].g * 255,
		(unsigned char) conf.col[RGBA_ERASE2].b * 255);
	/* map windows */
	toolwin_create();
	XMapWindow(dpy, win);
	XMapWindow(dpy, info->win);
	XFlush(dpy);
	spectro_draw();
	XSetWindowBackgroundPixmap(dpy, win, buf);
	XClearWindow(dpy,win);
	return 0;
}

int free_xlib() {
	toolwin_destroy();
	XFlush(dpy);
	XCloseDisplay(dpy);
}

cairo_t *xlib_context() {
	//XFillRectangle(dpy, buf, gc, 0, 0, ww, wh);
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
	XEvent ev;
	while (running && !XNextEvent(dpy,&ev))
		if (ev.type < LASTEvent && handler[ev.type])
			handler[ev.type](&ev);
}


