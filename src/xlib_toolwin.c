
typedef struct ToolWin ToolWin;
struct ToolWin {
	Window win;
	Pixmap buf;
	int w, h;
	char *name;
	cairo_t *ctx;
	Bool vis;
	int (*backing)(ToolWin *);
	int (*draw)(ToolWin *);
	int (*button)(ToolWin *);
};

static int toolwin_backing(ToolWin *);
static int toolwin_create();
static int toolwin_destroy();
static int toolwin_draw(ToolWin *);
static int toolwin_win_create(ToolWin *);
static int toolwin_win_destroy(ToolWin *);

static ToolWin *info, *help;
static char *help_name = "FEX Help";

#define MAX_STRING	256
#define A_LEFT		0
#define A_CENTER	1
#define A_RIGHT	2
int toolwin_printf(ToolWin *tw, int align, char *fmt, ...) {
	char str[MAX_STRING];
	va_list arg;
	va_start(arg, fmt);
	vsprintf(str, fmt, arg);
	va_end(arg);
	cairo_text_extents_t ext;
	cairo_text_extents(tw->ctx,str,&ext);
	double x = (align == A_RIGHT ? tw->w - ext.width - 2.0 :
			(align == A_CENTER ? (tw->w - ext.width) / 2.0 : 2.0));
	cairo_rel_move_to(tw->ctx, x, ext.height);
	cairo_show_text(tw->ctx,str);
	cairo_rel_move_to(tw->ctx, -1.0 * ext.width - x, ext.height / 2.0);
	return 0;
}

int info_draw(ToolWin *tw) {
	toolwin_backing(tw);
	cairo_move_to(tw->ctx, 0, 4);
	cairo_set_source_rgba(tw->ctx,0,0,0,1.0);
	toolwin_printf(tw, A_CENTER, "XXX sec, XXX KHz");
	toolwin_printf(tw, A_LEFT, "Threshold: %.3lf", -1.0 * conf.thresh);
	toolwin_draw(tw);
	return 0;
}

int toolwin_backing(ToolWin *tw) {
	cairo_set_source_rgba(tw->ctx,0.6,0.6,0.6,1.0);
	cairo_rectangle(tw->ctx, 0, 0, tw->w,tw->h);
	cairo_fill(tw->ctx);
	return 0;
}

int toolwin_create() {
	info = (ToolWin *) calloc(1, sizeof(ToolWin));
	help = (ToolWin *) calloc(1, sizeof(ToolWin));
	info->w = 200; info->h = 340;
	info->name = spect->name;
	info->vis = True;
	info->draw = info_draw;
	help->w = 320; help->h = 280;
	help->name = help_name;
	help->vis = False;
	toolwin_win_create(info);
	toolwin_win_create(help);
	return 0;
}

int toolwin_destroy() {
	toolwin_win_destroy(info);
	toolwin_win_destroy(help);
	free(info);
	free(help);
}

int toolwin_draw(ToolWin *tw) {
	XClearWindow(dpy,tw->win);
	XFlush(dpy);
	return 0;
}

int toolwin_win_create(ToolWin *tw) {
	tw->win = XCreateSimpleWindow(dpy, root, 20, 20, tw->w, tw->h, 0, 0, 0);
	tw->buf = XCreatePixmap(dpy, root, tw->w, tw->h, DefaultDepth(dpy,scr));
	cairo_surface_t *t = cairo_xlib_surface_create(dpy, tw->buf,
			DefaultVisual(dpy,scr), tw->w, tw->h);
	tw->ctx = cairo_create(t);
	cairo_surface_destroy(t);
	cairo_set_font_face(tw->ctx,conf.font);
	cairo_set_font_size(tw->ctx,conf.font_size);
	XSelectInput(dpy, tw->win, EVENT_MASK);
	XSetTransientForHint(dpy, tw->win, win);
	XSetWMProtocols(dpy, tw->win, &WM_DELETE_WINDOW, 1);
	XSizeHints *hints = XAllocSizeHints();
	hints->min_width = hints->max_width = tw->w;
	hints->min_height = hints->max_height = tw->h;
	hints->flags = PMinSize | PMaxSize;
	XSetWMNormalHints(dpy, tw->win, hints);
	XFree(hints);
	XStoreName(dpy, tw->win, tw->name);
	if (tw->backing) tw->backing(tw);
	else toolwin_backing(tw);
	XSetWindowBackgroundPixmap(dpy, tw->win, tw->buf);
	XFlush(dpy);
	return 0;
}

int toolwin_win_destroy(ToolWin *tw) {
	cairo_destroy(tw->ctx);
	XDestroyWindow(dpy, tw->win);
	XFreePixmap(dpy, tw->buf);
}

