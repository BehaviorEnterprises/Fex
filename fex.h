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
	int fs, ts;
	double **amp; /* [time][freq] */
	double *freq, *time;
} FFT;

typedef struct {
	const char *type;
	double a[4];
} SampleWindow;

extern int die(const char *);

extern int preview_create(int, int,FFT *);
extern int preview_peak(int,int);
extern int preview_test(long double, long double);
extern int preview_destroy();

extern int edit(int,int,double **);

double max, min, thresh, sp_floor;
double hipass, lopass;
int floor_num, floor_dem;
const char *name;
int range[2];

#endif /* __FEX_H__ */
