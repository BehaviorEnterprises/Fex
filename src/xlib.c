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

/* spectro.c */
extern int img_draw();
extern int spectro_draw();
extern int spectro_points();
extern int spectro_spec();
extern int spectro_thresh();

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
static int play(float);
static int pt_line(double, double);
static int screenshot();
static int sp_floor(double);
static int threshold(double);
static int zoom(double);

static Display *dpy;
static int scr;
static unsigned int ww, wh, mx, my;
static GC gc;
static Window root, win;
static Pixmap buf; //, ibuf, sbuf;
static Atom WM_DELETE_WINDOW;
//static cairo_t *ictx, *sctx;
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
#include "xlib_actions.c"
#include "xlib_events.c"

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
	XUndefineCursor(dpy, win);
	toolwin_destroy();
	XFlush(dpy);
	XCloseDisplay(dpy);
	return 0;
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
	XEvent ev;
	while (running && !XNextEvent(dpy,&ev))
		if (ev.type < LASTEvent && handler[ev.type])
			handler[ev.type](&ev);
	return 0;
}


