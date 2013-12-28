
#ifndef __FEX_H__
#define __FEX_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <sndfile.h>
#include <fftw3.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo.h>
#include <cairo-xlib.h>
#include <cairo-ft.h>

#define RGBA_SPECT	0x00
#define RGBA_THRESH	0x01
#define RGBA_POINTS	0x02
#define RGBA_LINES	0x03
#define RGBA_ERASE1	0x04
#define RGBA_ERASE2	0x05
#define RGBA_CROP		0x06
#define RGBA_LAST		0x07


#define set_color(x,n)	{	\
	cairo_set_source_rgba(x, conf.col[n].r, conf.col[n].g,	\
			conf.col[n].b, conf.col[n].a);							\
	cairo_set_line_width(x, conf.col[n].w);						\
}

typedef struct Wave {
	double *d;
	int samples;
	int rate;
} Wave;

typedef struct FFT {
	double **amp;
	double *time;
	double *freq;
	double max, min;
	int nfreq, ntime;
	char **mask;
} FFT;

typedef struct WindowFunction {
	const char *type;
	double a[4];
} WindowFunction;

typedef struct Spectro {
	const char *fname;
	char *name;
	unsigned char *a_spec, *a_thresh;
	cairo_surface_t *m_spec, *m_thresh, *s_points;
	FFT *fft;
	int fft_x, fft_y, fft_w, fft_h, fft_lo, fft_hi;
	double pex, tex, fex;
} Spectro;

typedef struct RGBA {
	double r, g, b, a, w;
} RGBA;

typedef struct Config {
	double thresh, spect_floor;
	double hipass, lopass;
	int scale;
	int winlen, hop, font_size;
	WindowFunction *win;
	RGBA col[RGBA_LAST];
	cairo_font_face_t *font, *bfont;
} Config;

/* main.c */
extern int die(const char *, ...);
/* config.c */
extern const char *configure(int, const char **);
/* fft.c */
extern FFT *create_fft(Wave *);
extern int free_fft(FFT **);
/* spectro.c */
extern int create_spectro(FFT *, const char *);
extern int free_spectro();
/* wave.c */
extern Wave *create_wave(const char *);
extern int free_wave(Wave **);
/* xlib.c */
extern int create_xlib();
extern int free_xlib();
extern cairo_t *xlib_context();
extern int xlib_event_loop();

/* global data */
Config conf;
Spectro *spect;

#endif /* __FEX_H__ */

