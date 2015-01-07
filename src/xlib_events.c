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
		/* The XServer is not storing the new/current values for width +
		 * height.  These must be retrieved directly.
		 * Is this an X11 bug? */
		Window wig;
		int ig;
		unsigned int uig;
		XGetGeometry(dpy, win, &wig, &ig, &ig, &ww, &wh, &uig, &uig);
		//ww = e->width; wh = e->height;
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
		else if (sym == XK_p) play(0.1666);
	}
	else if (mod == (Mod1Mask | ShiftMask)) {
		if (sym == XK_p) play(0.08333);
	}
	else if (mod == ControlMask) {
		if (sym == XK_q) running = False;
		else if (sym == XK_f) series_export();
		else if (sym == XK_s) screenshot();
		else if (sym == XK_i) img_draw();
		else if (sym == XK_j || sym == XK_Down) zoom(-0.025);
		else if (sym == XK_k || sym == XK_Up) zoom(0.025);
		else if (sym == XK_h || sym == XK_Left) return;
		else if (sym == XK_l || sym == XK_Right) return;
		else if (sym == XK_p) play(0.33);
	}
	else if (mod == Mod1Mask) {
		if (sym == XK_j || sym == XK_Down) eraser_cursor(-1,-1);
		else if (sym == XK_k || sym == XK_Up) eraser_cursor(1,1);
		else if (sym == XK_h || sym == XK_Left) eraser_cursor(-1,1);
		else if (sym == XK_l || sym == XK_Right) eraser_cursor(1,-1);
		else if (sym == XK_p) play(0.25);
	}
	else if (mod == ShiftMask) {
		if (sym == XK_j || sym == XK_Down) pt_line(-0.2,0);
		else if (sym == XK_k || sym == XK_Up) pt_line(0.2,0);
		else if (sym == XK_h || sym == XK_Left) pt_line(0,-0.2);
		else if (sym == XK_l || sym == XK_Right) pt_line(0,0.2);
		else if (sym == XK_p) play(0.5);
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
		//mode = MODE_ERASE & (mode ^= MODE_ERASE);
		mode = (mode & MODE_ERASE ? 0 : MODE_ERASE);
		eraser_cursor(0,0);
		info->draw(info);
	}
	else if (sym == XK_c) {
		//mode = MODE_CROP & (mode ^= MODE_CROP);
		mode = (mode & MODE_CROP ? 0 : MODE_CROP);
		if (!(mode & MODE_CROP)) XDefineCursor(dpy, win, None);
		else XDefineCursor(dpy, win, XCreateFontCursor(dpy, 34));
		info->draw(info);
	}
	else if (sym == XK_Escape) {
		mode = MODE_NULL;
		XDefineCursor(dpy, win, None);
		info->draw(info);
	}
	else if (sym == XK_p) play(1.0);
	else if (sym == XK_t) {
		conf.layers = !conf.layers;
		spectro_draw();
		XCopyArea(dpy, buf, win, gc, 0, 0, ww, wh, 0, 0);
	}
	else if (sym == XK_u && mode & (MODE_ERASE)) erase(-1,-1);
	while(XCheckMaskEvent(dpy, KeyPressMask, ev));
}

void motionnotify(XEvent *ev) {
	//static int px, py;
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

