
static Window iwin;
static Pixmap ibuf;
static cairo_t *ictx;
static Bool ivis = True;

int info_draw() {
	XClearWindow(dpy,iwin);
	//cairo_set_source_rgba(ictx,0.5,0.5,0.5,1.0);
	//cairo_rectangle(ictx,0,0,IWIN_W,IWIN_H);
	//cairo_fill(ictx);
	//XCopyArea(dpy, ibuf, iwin, gc, 0, 0, ww, wh, 0, 0);
	XFlush(dpy);
}

int info_backing() {
	/* background */
	cairo_set_source_rgba(ictx,0.6,0.6,0.6,1.0);
	cairo_rectangle(ictx,0,0,IWIN_W,IWIN_H);
	cairo_fill(ictx);
}

void info_buttonpress(XButtonEvent *e) {
//	if (e->button == 1) { /* move window */
//		XGrabPointer(dpy, win, True, PointerMotionMask | ButtonReleaseMask,
//				GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
//		XEvent ee;
//		int dx, dy;
//		int xx = e->x_root, yy = e->y_root;
//		int wx, wy, ig;
//		Window w;
//		XGetGeometry(dpy, iwin, &w, &wx, &wy, &ig, &ig, &ig, &ig);
//		while (True) {
//			XMaskEvent(dpy, PointerMotionMask | ButtonReleaseMask, &ee);
//			if (ee.type == ButtonRelease) break;
//			dx = ee.xbutton.x_root - xx;
//			dy = ee.xbutton.y_root - yy;
//			XMoveWindow(dpy, iwin, wx+dx, wy+dy);
//			XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
//			info_draw();
//		}
//		XUngrabPointer(dpy,CurrentTime);
//		info_draw();
//	}
}

int info_create() {
	iwin = XCreateSimpleWindow(dpy,root, 20, 20, IWIN_W, IWIN_H, 0, 0, 0);
	ibuf = XCreatePixmap(dpy,root,IWIN_W,IWIN_H,DefaultDepth(dpy,scr));
	cairo_surface_t *t = cairo_xlib_surface_create(dpy, ibuf,
			DefaultVisual(dpy,scr), IWIN_W, IWIN_H);
	ictx = cairo_create(t);
	cairo_surface_destroy(t);
	XSelectInput(dpy, iwin, EVENT_MASK);
	XSetTransientForHint(dpy, iwin, win);
	XSetWMProtocols(dpy, iwin, &WM_DELETE_WINDOW, 1);

	XSizeHints *hints = XAllocSizeHints();
	hints->min_width = hints->max_width = IWIN_W;
	hints->min_height = hints->max_height = IWIN_H;
	hints->flags = PMinSize | PMaxSize;
	XSetWMNormalHints(dpy, iwin, hints);
	XFree(hints);

	XStoreName(dpy, iwin, spect->name);
	info_backing();
	XSetWindowBackgroundPixmap(dpy, iwin, ibuf);
	XFlush(dpy);
}	

int info_destroy() {
	cairo_destroy(ictx);
	XDestroyWindow(dpy, iwin);
	XFreePixmap(dpy, ibuf);
}
