
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
	int (*button)(ToolWin *, XButtonEvent *);
};

static int toolwin_backing(ToolWin *);
static int toolwin_button(ToolWin *, XButtonEvent *);
static int toolwin_create();
static int toolwin_destroy();
static int toolwin_draw(ToolWin *);
static int toolwin_win_create(ToolWin *);
static int toolwin_win_destroy(ToolWin *);

static ToolWin *info; //, *help;
//static char *help_name = "FEX Help";
static cairo_text_extents_t ext;


//static int help_draw(ToolWin *tw) {
//	toolwin_backing(tw);
//	cairo_set_source_rgba(tw->ctx,0,0,0,1.0);
//	cairo_rectangle(tw->ctx, 10, 10, 460, 300);
//	cairo_stroke_preserve(tw->ctx);
//	cairo_set_source_rgba(tw->ctx,0.8,0.8,0.8,1.0);
//	cairo_fill(tw->ctx);
////	cairo_surface_t *img = cairo_image_surface_create_from_png(
////			"./help.png");
////	cairo_set_source_surface(tw->ctx,img, 8, 8);
////	cairo_paint(tw->ctx);
////	cairo_surface_destroy(img);
//
//	toolwin_draw(tw);
//}

#define MARGIN		4.0
#define MAX_STRING	256
#define A_LEFT		0x00
#define A_CENTER	0x01
#define A_RIGHT	0x02
static int info_printf(ToolWin *tw, int align, char *fmt, ...) {
	char str[MAX_STRING];
	va_list arg;
	va_start(arg, fmt);
	vsprintf(str, fmt, arg);
	va_end(arg);
	cairo_text_extents(tw->ctx,str,&ext);
	double x = (align == A_RIGHT ? tw->w - ext.x_advance - MARGIN :
			(align == A_CENTER ? (tw->w - ext.x_advance) / 2.0 : MARGIN));
	cairo_rel_move_to(tw->ctx, x, conf.font_size);
	cairo_show_text(tw->ctx,str);
	cairo_rel_move_to(tw->ctx, -1.0 * ext.x_advance - x, conf.font_size / 3.0);
	return 0;
}

static int info_split_printf(ToolWin *tw, char *bold, char *fmt, ...) {
	char str[MAX_STRING];
	va_list arg;
	va_start(arg, fmt);
	vsprintf(str, fmt, arg);
	va_end(arg);
	cairo_set_font_face(tw->ctx, conf.bfont);
	cairo_text_extents(tw->ctx,bold,&ext);
	cairo_rel_move_to(tw->ctx, MARGIN, conf.font_size);
	cairo_show_text(tw->ctx,bold);
	cairo_rel_move_to(tw->ctx, -1.0 * ext.x_advance - MARGIN, 0);
	cairo_set_font_face(tw->ctx, conf.font);
	cairo_text_extents(tw->ctx,str,&ext);
	cairo_rel_move_to(tw->ctx, tw->w - ext.x_advance - MARGIN, 0);
	cairo_show_text(tw->ctx,str);
	cairo_rel_move_to(tw->ctx, MARGIN - tw->w, conf.font_size / 3.0);
	return 0;
}

static int info_draw(ToolWin *tw) {
	toolwin_backing(tw);
	/* info */
	cairo_set_source_rgba(tw->ctx,0,0,0,1.0);
	cairo_set_font_face(tw->ctx, conf.bfont);
	cairo_move_to(tw->ctx, 0, 8);
	info_printf(tw, A_CENTER, "%.3lf sec, %.2lf KHz",
			spect->fft->time[mx],spect->fft->freq[my]);
	cairo_rel_move_to(tw->ctx,0,12);
	info_split_printf(tw, "Threshold:", "%.2lf", -1.0 * conf.thresh);
	info_split_printf(tw, "Spectrogram Floor:", "%.2lf",
			-1.0 * conf.spect_floor);
	info_split_printf(tw, "Path Length:","%.2lf", spect->pex);
	info_split_printf(tw, "Path Duration:","%.3lf", spect->tex);
	info_split_printf(tw, "Frequency Excursion:","%.3lf", spect->fex);
	cairo_move_to(tw->ctx, 0, 145);
	/* modes */
	info_split_printf(tw, "Modes:","");
	cairo_set_font_face(tw->ctx, conf.bfont);
		/* erase */
	set_color(tw->ctx,RGBA_ERASE1)
	if (mode & MODE_ERASE) set_color(tw->ctx,RGBA_ERASE1)
	else cairo_set_source_rgba(tw->ctx,0.8,0.8,0.8,1.0);
	cairo_rectangle(tw->ctx, 10, 170, tw->w / 2.0 - 15, 20);
	cairo_fill_preserve(tw->ctx);
	cairo_set_source_rgba(tw->ctx,0,0,0,1.0);
	cairo_stroke(tw->ctx);
	cairo_text_extents(tw->ctx,"Erase",&ext);
	cairo_move_to(tw->ctx, tw->w / 4.0 - ext.width / 2.0, 185);
	cairo_set_source_rgba(tw->ctx,0,0,0,1.0);
	cairo_show_text(tw->ctx,"Erase");
		/* crop */
	if (mode & MODE_CROP) set_color(tw->ctx, RGBA_CROP)
	else cairo_set_source_rgba(tw->ctx,0.8,0.8,0.8,1.0);
	cairo_rectangle(tw->ctx, 5 + tw->w / 2.0, 170, tw->w / 2.0 - 15, 20);
	cairo_fill_preserve(tw->ctx);
	cairo_set_source_rgba(tw->ctx,0.0,0.0,0.0,1.0);
	cairo_stroke(tw->ctx);
	cairo_text_extents(tw->ctx,"Crop",&ext);
	cairo_move_to(tw->ctx, tw->w * 0.75 - ext.width / 2.0, 185);
	cairo_set_source_rgba(tw->ctx,0,0,0,1.0);
	cairo_show_text(tw->ctx,"Crop");
	/* draw */
	toolwin_draw(tw);
	return 0;
}

static int info_button(ToolWin *tw, XButtonEvent *e) {
	if (e->y < 170 || e->y > 190) return;
	if (e->x < 10 || e->x > tw->w - 10) return;
	if (e->x < tw->w / 2.0 - 5) { /* erase button */
		if (e->button == 1) {
			mode = MODE_ERASE & (mode ^= MODE_ERASE);
			eraser_cursor(0,0);
			tw->draw(tw);
		}
	}
	else { /* crop button */
		if (e->button == 1) {
			mode = MODE_CROP & (mode ^= MODE_CROP);
			if (!(mode & MODE_CROP)) XDefineCursor(dpy, win, None);
			else XDefineCursor(dpy, win, XCreateFontCursor(dpy, 34));
			tw->draw(tw);
		}
		else if (e->button == 3) { /* uncrop */
			mode |= MODE_CROP;
			tw->draw(tw);
			spect->fft_x = 0;
			spect->fft_y = spect->fft_lo;
			spect->fft_w = spect->fft->ntime;
			spect->fft_h = spect->fft_hi - spect->fft_lo;
			spectro_spec();
			spectro_thresh();
			spectro_points();
			spectro_draw();
			XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
			mode &= ~MODE_CROP;
			tw->draw(tw);
		}
	}
}

int toolwin_backing(ToolWin *tw) {
	cairo_set_source_rgba(tw->ctx,0.6,0.6,0.6,1.0);
	cairo_rectangle(tw->ctx, 0, 0, tw->w,tw->h);
	cairo_fill(tw->ctx);
	return 0;
}

int toolwin_button(ToolWin *tw, XButtonEvent *e) {
	XRaiseWindow(dpy, tw->win);
}

int toolwin_create() {
	info = (ToolWin *) calloc(1, sizeof(ToolWin));
	//help = (ToolWin *) calloc(1, sizeof(ToolWin));
	info->w = 240; info->h = 200;
	info->name = spect->name;
	info->vis = True;
	info->draw = info_draw;
	info->button = info_button;
	//help->w = 480; help->h = 320;
	//help->name = help_name;
	//help->vis = False;
	//help->draw = help_draw;
	//help->button = toolwin_button;
	toolwin_win_create(info);
	//toolwin_win_create(help);
	return 0;
}

int toolwin_destroy() {
	toolwin_win_destroy(info);
	//toolwin_win_destroy(help);
	free(info);
	//free(help);
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

