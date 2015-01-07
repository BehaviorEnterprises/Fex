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
	spect->fft_h = spect->fft_h*(1.0-y2/(wh*ysc))-1.0*yoff +
			oldy - spect->fft_y;
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
	return 0;
}

int eraser_cursor(int w, int h) {
	XUndefineCursor(dpy, win);
	if ( !(mode & MODE_ERASE) ) return 0;
	if ( (ew+=w) < 3 ) ew = 3;
	if ( (eh+=h) < 3 ) eh = 3;
	if ( ew > ww/4 ) ew = ww / 4;
	if ( eh > wh/2 ) eh = wh / 2;
	int i, stride = ew/ 8 + 1;
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
	return 0;
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
	return 0;
}

int play(float speed) {
	/* calculate fft coordinates of current view window */
	int tw = spect->fft_w / xsc;
	int th = spect->fft_h / ysc;
	int tx = spect->fft_x - xoff;
	int ty = spect->fft_y - yoff + spect->fft_h - th;
	if (tx + tw >= spect->fft->ntime) tw = spect->fft->ntime - tx - 1;
	if (ty + th >= spect->fft->nfreq) th = spect->fft->nfreq - ty - 1;
	/* get time and frequency values of current view window */
	double _start = spect->fft->time[tx];
	double _end = spect->fft->time[tx+tw];
	double _low = spect->fft->freq[ty];
	double _hi = spect->fft->freq[ty+th];
	double _mid = (_hi+_low)/2.0;
	double _wid = (_hi-_low)/2.0;
	/* fork and play */
	if (!fork()==0) {
		close(ConnectionNumber(dpy));
		char start[12], end[12], mid[12], wid[12], sp[8];
		snprintf(sp,7,"%3f",speed);
		sprintf(start,"=%lf",_start); sprintf(end,"=%lf",_end);
		sprintf(mid,"%lfk",_mid); sprintf(wid,"%lfk",_wid);
		execl("/usr/bin/play", "play", "-q", spect->fname,
				"trim", start, end, "bandpass", mid, wid,
				"speed", sp, NULL);
		perror("Fex Play");
		exit(1);
	}
	/* draw cursor while playing */
	int i, step = 12;
	double dur = _end - _start;
	useconds_t usec = dur * 1000000.0 * step / (double) ww;
	step = speed * step + 0.5;
	for (i = 0; i < ww; i+=step) {
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
		XDrawLine(dpy, win, gc, i, 0, i, wh);
		XDrawLine(dpy, win, gc, i+step, 0, i+step, wh);
		XFlush(dpy);
		usleep(usec);
	}
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	XFlush(dpy);
	return 0;
}

int pt_line(double p, double l) {
	conf.col[RGBA_POINTS].w += p;
	conf.col[RGBA_LINES].w += l;
	if (conf.col[RGBA_POINTS].w < 0) conf.col[RGBA_POINTS].w = 0.0;
	if (conf.col[RGBA_LINES].w < 0) conf.col[RGBA_LINES].w = 0.0;
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	return 0;
}

int screenshot() {
	char fname[256];
	static int n = 0;
	snprintf(fname, 256, "%s-%d.png", spect->name, n++);
	cairo_surface_t *t = cairo_xlib_surface_create(dpy, buf,
			DefaultVisual(dpy,scr), ww, wh);
	cairo_surface_write_to_png(t, fname);
	cairo_surface_destroy(t);
	return 0;
}

int series_export() {
	double *dat = malloc(spect->fft_w * sizeof(double));
	int i, j, f;
	/* find the peak frequency relative to max for each time bin */
	for (i = spect->fft_x; i < spect->fft_w + spect->fft_x; ++i) {
		f = 0;
		for (j = spect->fft_y; j < spect->fft_h+spect->fft_y; ++j) {
				if (spect->fft->mask[i][j]) continue;
				if (spect->fft->amp[i][j] > spect->fft->amp[i][f] || !f)
					f = j;
		}
		if (f > 0 && spect->fft->amp[i][f] > conf.thresh)
			dat[i - spect->fft_x] = spect->fft->freq[f];
		else
			dat[i - spect->fft_x] = 0.0;
	}
	char *fname = malloc(strlen(spect->name) + 6);
	sprintf(fname, "%s.freq", spect->name);
	f = open(fname, O_WRONLY | O_CREAT, 0644);
	write(f, dat, spect->fft_w * sizeof(double));
	close(f);
	free(fname);
	free(dat);
	return 0;
}

int sp_floor(double f) {
	conf.spect_floor += f;
	spectro_spec();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	if (info->vis) info->draw(info);
	return 0;
}

int threshold(double f) {
	conf.thresh += f;
	spectro_thresh();
	spectro_points();
	spectro_draw();
	XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	if (info->vis) info->draw(info);
	return 0;
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
	return 0;
}

