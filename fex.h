/* This file is part of FEX, copyright Jesse McClure 2013 */
/* See COPYING for license information */

#ifndef __FEX_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sndfile.h>
#include <fftw3.h>
#include <math.h>

#define TMP_FILE	"/tmp/feh.png"

typedef struct {
	double *d;
	int samples;
	int rate;
} Wave;

typedef struct {
	double **amp; /* [time][freq] */
	double *freq, *time;
	int fs, ts;
} FFT;

typedef struct {
	const char *type;
	double a[4];
} SampleWindow;

typedef struct {int x1,y1,x2,y2; } ZRect;

extern int die(const char *);

extern int preview_create(int, int,FFT *);
extern int preview_peak(int,int);
extern int preview_test(long double, long double);
extern int preview_destroy();

extern int edit(int,int,double **);

double max, min, thresh;
const char *name;
ZRect zrect;

#endif /* __FEX_H__ */
