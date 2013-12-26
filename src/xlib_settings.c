
static Window swin;
static Pixmap sbuf;
static cairo_t *sctx;
static Bool svis = False;

int settings_draw() {
	XClearWindow(dpy,swin);
	XFlush(dpy);
}

int settings_backing() {
	/* background */
	cairo_set_source_rgba(sctx,0.6,0.6,0.6,1.0);
	cairo_rectangle(sctx,0,0,SWIN_W,SWIN_H);
	cairo_fill(sctx);
	/*rects*/
	cairo_set_line_width(sctx,1);
	int i;
	const char *opt[6] = {
		"spectrogram", "threshold", "points",
		"lines", "eraser", "zoomer" };
	char num[6];
	for (i = 0; i < 6; i++) {
		cairo_rectangle(sctx,10,80 + 25*i,34,20);
		cairo_set_source_rgba(sctx,1.0,1.0,1.0,1.0);
		cairo_fill_preserve(sctx);
		cairo_set_source_rgba(sctx, conf.col[i].r, conf.col[i].g,
				conf.col[i].b, conf.col[i].a);
		cairo_fill_preserve(sctx);
		cairo_set_source_rgba(sctx,0.0,0.0,0.0,1.0);
		cairo_stroke(sctx);
		cairo_set_source_rgba(sctx, 1.0 - conf.col[i].r,
				1.0 - conf.col[i].g, 1.0 - conf.col[i].b, 1.0);
		snprintf(num,5,"%lf",conf.col[i].w);
		cairo_move_to(sctx,16,95+25*i);
		cairo_show_text(sctx,num);
		cairo_set_source_rgba(sctx,0.0,0.0,0.0,1.0);
		cairo_move_to(sctx,52,95+25*i);
		cairo_show_text(sctx,opt[i]);
	}
	XFlush(dpy);
}

void settings_buttonpress(XButtonEvent *e) {

}

int settings_create() {
	swin = XCreateSimpleWindow(dpy,root, 80, 80, SWIN_W, SWIN_H, 0, 0, 0);
	sbuf = XCreatePixmap(dpy,root,SWIN_W,SWIN_H,DefaultDepth(dpy,scr));
	cairo_surface_t *t = cairo_xlib_surface_create(dpy, sbuf,
			DefaultVisual(dpy,scr), SWIN_W, SWIN_H);
	sctx = cairo_create(t);
	cairo_surface_destroy(t);
	XSelectInput(dpy, swin, EVENT_MASK);
	XSetTransientForHint(dpy, swin, win);
	XStoreName(dpy, swin, "FEX Settings");
	XSetWMProtocols(dpy, swin, &WM_DELETE_WINDOW, 1);
	settings_backing();
	XSetWindowBackgroundPixmap(dpy, swin, sbuf);
}	

int settings_destroy() {
	cairo_destroy(sctx);
	XDestroyWindow(dpy, swin);
	XFreePixmap(dpy,sbuf);
}
