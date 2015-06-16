
#include <math.h>
#include "fex.h"
#include "fex_struct.h"

extern fex_t read_wave(FEX *);
extern fex_t free_wave(FEX *);
extern fex_t configure(FEX *);
extern fex_t create_fft(FEX *);
extern fex_t destroy_fft(FEX *);

static fex_t fex_populate(FEX *);
static fex_t fex_depopulate(FEX *);
static FEX *fex_ptr(fex_t);
static int fex_make_path(FEX *);

static unsigned char *spec_data = NULL;
static FEX **fex_p = NULL;
static int nfex = 0;


/**********************/
/* exported functions */
/**********************/

fex_t fex_create(const char *fname) {
	fex_p = realloc(fex_p, (++nfex) * sizeof(FEX *));
	fex_p[nfex-1] = malloc(sizeof(FEX));
	fex_t ret;
	if (fex_p[nfex-1]) {
		fex_p[nfex-1]->wave.name = fname;
		ret = fex_populate(fex_p[nfex-1]);
		if (ret < 0) return ret;
		else return nfex-1;
	}
	return FexMallocError;
}

fex_t fex_destroy(fex_t fex_n) {
	if (spec_data) free(spec_data);
	spec_data = NULL;
	fex_t ret;
	if ( (ret=fex_depopulate(fex_p[fex_n])) < 0 ) return ret;
	free(fex_p[fex_n]);
	fex_p[fex_n] = NULL;
	return ret;
}

#define square(x)  (x)*(x)
unsigned char *fex_get_spectrogram(fex_t fex_n, int *w_ptr, int *h_ptr) {
	FEX *fex = fex_ptr(fex_n);
	if (!fex) return NULL;
	int w = fex->fft.ntime;
	int h = fex->fft.nfreq;

	spec_data = realloc(spec_data, w * h * 3);
	// TODO error checking, speed of loop order?
	int t, f;
	unsigned char *a = spec_data;
unsigned char adj = 0.5 + 255.0 * fex->conf.threshold / fex->conf.floor;
	for (f = 0; f < h; ++f) for (t = 0; t < w; ++t) {
// TODO clean up!
//double dd = square(-sqrt(255.0) * fftAmp(fex,t,f) / fex->conf.floor);
double dd =  255.0 * fftAmp(fex,t,f) / fex->conf.floor;
unsigned char aa = dd;
if (dd > 255) aa = 255;
if (dd < 0) aa = 0;
		a[0] = a[1] = a[2] = aa;
		if (aa < adj) a[1] = a[2] = aa + 255-adj;
		a += 3;
	}
	*w_ptr = w;
	*h_ptr = h;
	fex_make_path(fex);
	return spec_data;
}

int *fex_get_path(fex_t fex_n) {
	FEX *fex = fex_ptr(fex_n);
	return fex->fft.path;
}

int fex_threshold(fex_t fex_n, float adj) {
	FEX *fex = fex_ptr(fex_n);
	fex->conf.threshold += adj;
	fex_make_path(fex);
	return 0;
}

#define dist(x,y)     sqrt((x)*(x)+(y)*(y))
double fex_measure(fex_t fex_n, double *len_ptr, double *duration_ptr) {
	FEX *fex = fex_ptr(fex_n);
	int t, f;
	double prev_t = -1.0, prev_f, start, end;
	long double sum = 0.0;
	for (t = 0; t < fex->fft.ntime; ++t) {
		if ( (f=fex->fft.path[t]) < 0 ) continue;
		if (prev_t > 0)
			sum += dist(fex->fft.time[t] - prev_t, fex->fft.freq[f] - prev_f);
		else
			start = fex->fft.time[t];
		prev_t = fex->fft.time[t];
		prev_f = fex->fft.freq[f];
		end = fex->fft.time[t];
	}
	if (len_ptr) *len_ptr = sum;
	if (duration_ptr) *duration_ptr = end - start;
	return sum / (end - start);
}


int fex_coordinates(fex_t fex_n, double pt, double pf, double *time_ptr, double *freq_ptr) {
	FEX *fex = fex_ptr(fex_n);
	if (time_ptr) *time_ptr = fex->fft.time[(int) (pt * fex->fft.ntime)];
	if (freq_ptr) *freq_ptr = fex->fft.freq[(int) (pf * fex->fft.nfreq)];
	return 0;
}

/*******************/
/* local functions */
/*******************/


int fex_make_path(FEX *fex) {
	int t, f;
	double max;
	for (t = 0; t < fex->fft.ntime; ++t) {
		max = -1.0 * INFINITY;
		fex->fft.path[t] = -1;
		for (f = 0; f < fex->fft.nfreq; ++f) {
			if (fftAmp(fex,t,f) < fex->conf.threshold)
				continue;
			if (fftAmp(fex,t,f) > max)
				max = fftAmp(fex,t, (fex->fft.path[t]=f));
		}
	}
	return 0;
}


FEX *fex_ptr(fex_t fex_n) {
	if (fex_n >= nfex) return NULL;
	else return fex_p[fex_n];
}

fex_t fex_populate(FEX *fex) {
	if (!fex) return FexNullAccess;
	fex_t ret;
	if ( (ret=read_wave(fex)) < 0 ) return ret;
	if ( (ret=configure(fex)) < 0 ) return ret;
	if ( (ret=create_fft(fex)) < 0 ) return ret;
	fex->fft.path = malloc(fex->fft.ntime * sizeof(int));
}

fex_t fex_depopulate(FEX *fex) {
	if (!fex) return FexNullAccess;
	fex_t ret;
	if (fex->fft.path) free(fex->fft.path);
	fex->fft.path = NULL;
	if ( (ret=destroy_fft(fex)) < 0 ) return ret;
	if ( (ret=free_wave(fex)) < 0 ) return ret;
}


