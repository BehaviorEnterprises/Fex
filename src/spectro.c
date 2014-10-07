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

#define set_color(x,n)	{	\
	cairo_set_source_rgba(x, conf.col[n].r, conf.col[n].g,	\
			conf.col[n].b, conf.col[n].a);							\
	cairo_set_line_width(x, conf.col[n].w);						\
}

int img_draw() {
	cairo_surface_t *img;
	img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, spect->fft_w*conf.scale, spect->fft_h*conf.scale * 4);
	cairo_t *ctx;
	ctx = cairo_create(img);
	cairo_scale(ctx,conf.scale,-4.0 * conf.scale);
	cairo_translate(ctx, 0, -1.0 * spect->fft_h);
	cairo_set_source_rgba(ctx,1.0,1.0,1.0,1.0);
	cairo_rectangle(ctx,0,0,spect->fft_w,spect->fft_h);
	cairo_fill(ctx);
	set_color(ctx,RGBA_SPECT);
	cairo_mask_surface(ctx,spect->m_spec,0,0);
	cairo_surface_write_to_png(img,"/tmp/img1.png");
	cairo_destroy(ctx);
	cairo_surface_destroy(img);
	img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, spect->fft_w*conf.scale, spect->fft_h*conf.scale * 4);
	ctx = cairo_create(img);
	cairo_scale(ctx,conf.scale,-4.0 * conf.scale);
	cairo_translate(ctx, 0, -1.0 * spect->fft_h);
	cairo_set_source_rgba(ctx, conf.col[RGBA_THRESH].r, conf.col[RGBA_THRESH].g,
		conf.col[RGBA_THRESH].b, 0.8);
	cairo_mask_surface(ctx,spect->m_thresh,0,0);
	cairo_surface_write_to_png(img,"/tmp/img2.png");
	cairo_destroy(ctx);
	cairo_surface_destroy(img);
	img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, spect->fft_w*conf.scale, spect->fft_h*conf.scale * 4);
	ctx = cairo_create(img);
	cairo_scale(ctx, 1.0, -4.0);
	cairo_translate(ctx, 0, -1.0 * spect->fft_h * conf.scale);
	cairo_set_source_surface(ctx,spect->s_points, 0, 0);
	cairo_paint_with_alpha(ctx, 0.8);
	cairo_surface_write_to_png(img,"/tmp/img3.png");
	cairo_destroy(ctx);
	cairo_surface_destroy(img);
	return 0;
}

int spectro_draw() {
	cairo_t *c = xlib_context();
	cairo_set_source_rgba(c,1.0,1.0,1.0,1.0);
	cairo_rectangle(c,0,0,spect->fft_w,spect->fft_h);
	cairo_fill(c);
	set_color(c,RGBA_SPECT);
	cairo_mask_surface(c,spect->m_spec,0,0);
	if (conf.layers) {
		set_color(c,RGBA_THRESH);
		cairo_mask_surface(c,spect->m_thresh,0,0);
		cairo_scale(c,1.0/conf.scale,1.0/conf.scale);
		cairo_set_source_surface(c,spect->s_points,0,0);
		cairo_paint(c);
	}
	cairo_destroy(c);
	return 0;
}

int create_spectro(FFT *fft, const char *fname) {
	spect = (Spectro *) calloc(1, sizeof(Spectro));
	/* names */
	spect->fname = fname;
	char *c = strrchr(spect->fname,'/');
	if (c && (++c)) spect->name = strdup(c);
	else spect->name = strdup(fname);
	if ( (c=strrchr(spect->name,'.')) ) *c = '\0';
	/* fft */
	spect->fft = fft;
	spect->fft_w = fft->ntime;
	spect->fft_h = fft->nfreq;
	/* set bounds */
	int i;
	for (i = 0; i < spect->fft->nfreq &&
			spect->fft->freq[i] < conf.hipass; i++);
	spect->fft_lo = spect->fft_y = i;
	for (i++; i < spect->fft->nfreq &&
			spect->fft->freq[i] < conf.lopass; i++);
	spect->fft_hi = i;
	spect->fft_h = i - spect->fft_y;
	/* cairo_surfaces */
	spectro_spec();
	spectro_thresh();
	spectro_points();
	/* xlib */
	create_xlib();
	return 0;
}

int free_spectro() {
	cairo_surface_destroy(spect->m_spec);
	cairo_surface_destroy(spect->m_thresh);
	cairo_surface_destroy(spect->s_points);
	free_xlib();
	free(spect->name);
	free(spect);
	return 0;
}

int spectro_spec() {
	if (spect->m_spec) cairo_surface_destroy(spect->m_spec);
	if (spect->a_spec) free(spect->a_spec);
	int i, j, stride;
	stride = cairo_format_stride_for_width(CAIRO_FORMAT_A8, spect->fft_w);
	spect->a_spec = (unsigned char *) calloc(stride,spect->fft_h);
	unsigned char *a = NULL;
	for (j = spect->fft_y; j < spect->fft_h + spect->fft_y; j++) {
		a = spect->a_spec + (j - spect->fft_y) * stride;
		for (i = spect->fft_x; i < spect->fft_w + spect->fft_x; i++, a++) {
			*a = 255  - (unsigned char) 255 * (
					( (spect->fft->amp[i][j] - conf.spect_floor) >= 0) ?
						spect->fft->amp[i][j] / conf.spect_floor : 1);
		}
	}
	spect->m_spec = cairo_image_surface_create_for_data(spect->a_spec,
			CAIRO_FORMAT_A8, spect->fft_w, spect->fft_h, stride);
	return 0;
}

int spectro_thresh() {
	if (spect->m_thresh) cairo_surface_destroy(spect->m_thresh);
	if (spect->a_thresh) free(spect->a_thresh);
	int i, j, stride;
	stride = cairo_format_stride_for_width(CAIRO_FORMAT_A8, spect->fft_w);
	spect->a_thresh = (unsigned char *) calloc(stride, spect->fft_h);
	unsigned char *a = NULL;
	for (j = spect->fft_y; j < spect->fft_h + spect->fft_y; j++) {
		a = spect->a_thresh + (j - spect->fft_y) * stride;
		for (i = spect->fft_x; i < spect->fft_w + spect->fft_x; i++, a++) {
			if (spect->fft->mask[i][j]) *a = 0;
			else if (spect->fft->amp[i][j] > conf.thresh) *a = 255;
			else *a = 0;
		}
	}
	spect->m_thresh = cairo_image_surface_create_for_data(spect->a_thresh,
			CAIRO_FORMAT_A8, spect->fft_w, spect->fft_h, stride);
	return 0;
}

int spectro_points() {
	if (spect->s_points) cairo_surface_destroy(spect->s_points);
	spect->s_points = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			//spect->fft_w, spect->fft_h);
			spect->fft_w * conf.scale, spect->fft_h * conf.scale);
	cairo_t *p = cairo_create(spect->s_points);
	cairo_t *l = cairo_create(spect->s_points);
	set_color(p,RGBA_POINTS);
	set_color(l,RGBA_LINES);
	int i, j, f;
	double lt = spect->fft->time[0], lf = spect->fft->freq[0];
	spect->pex = 0.0;
	spect->tex = 0.0;
	/* loop through time bins */
	for (i = spect->fft_x; i < spect->fft_w + spect->fft_x; i++) {
		/* find maximum (non masked) frequency in time bin */
		for (f = 0, j = spect->fft_y; j < spect->fft_h+spect->fft_y; j++) {
				if (spect->fft->mask[i][j]) continue;
				if (spect->fft->amp[i][j] > spect->fft->amp[i][f] || !f)
					f = j;
		}
		/* add points and do calculations if f is above threshold */
		if (f > 0 && spect->fft->amp[i][f] > conf.thresh) {
			if (lt != spect->fft->time[0]) {
				spect->pex += sqrt(
					(spect->fft->freq[f] - lf) * (spect->fft->freq[f] - lf) +
					(spect->fft->time[i] - lt) * (spect->fft->time[i] - lt) );
				spect->tex += spect->fft->time[i] - lt;
			}
			lt = spect->fft->time[i];
			lf = spect->fft->freq[f];
			cairo_line_to(l,
					(i - spect->fft_x) * conf.scale + conf.scale / 2,
					(f - spect->fft_y) * conf.scale + conf.scale / 2);
			cairo_new_sub_path(p);
			cairo_arc(p,
					(i - spect->fft_x) * conf.scale + conf.scale / 2,
					(f - spect->fft_y) * conf.scale + conf.scale / 2,
					conf.col[RGBA_POINTS].w,0,2*M_PI);
		}
		spect->fex = spect->pex / spect->tex;
	}
	cairo_fill(p);
	cairo_stroke(l);
	cairo_destroy(p);
	cairo_destroy(l);
	return 0;
}
