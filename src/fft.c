/**********************************************************************\
* FFT.C
* Part of FEX, see FEX.C for complete license information
* Author: Jesse McClure, copyright 2013-2015
* License; GPL3
\**********************************************************************/

#include <math.h>
#include <fftw3.h>
#include "fex.h"
#include "fex_struct.h"

// TODO configurable window_function
static double window_function[4] = { 0.5, 0.5, 0, 0 };


//#define MatrixAmp(mat,nf,t,f)		(mat)[nf * t + f]
fex_t create_fft(FEX *fex) {
	if (!fex) return FexNullAccess;
	/* allocate space */
	fex->fft.nfreq = fex->conf.winlen / 2 + 0.5;
	//fex->fft.ntime = (fex->wave.samples - fex->conf.winlen) / fex->conf.hop;
	fex->fft.ntime = fex->wave.samples / fex->conf.hop;
	fex->fft.freq = malloc(fex->fft.nfreq * sizeof(double));
	fex->fft.time = malloc(fex->fft.ntime * sizeof(double));
	fex->fft.amp = malloc(fex->fft.nfreq * fex->fft.ntime * sizeof(double));
	/* fill time and freq arrays */
	double nyquist = (double) fex->wave.rate / 2000.0;
	double dt = fex->wave.samples / (double) (fex->wave.rate * fex->fft.ntime);
	double df = nyquist / fex->fft.nfreq;
	int i;
	fex->fft.time[0] = fex->fft.freq[0] = 0.0;
	for (i = 1; i < fex->fft.ntime; ++i)
		fex->fft.time[i] = fex->fft.time[i-1] + dt;
	for (i = 1; i < fex->fft.nfreq; ++i)
		fex->fft.freq[i] = fex->fft.freq[i-1] + df;
	/* preare fftw */
	fftw_complex *in, *out;
	fftw_plan plan;
	in = (fftw_complex *) fftw_malloc(fex->conf.winlen * sizeof(fftw_complex));
	out = (fftw_complex *) fftw_malloc(fex->conf.winlen * sizeof(fftw_complex));
	plan = fftw_plan_dft_1d(fex->conf.winlen, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	/* windowing function */
	double *window = malloc(fex->conf.winlen * sizeof(double));
	int t, f, pos;
	for (t = 0; t < fex->conf.winlen; ++t)
		window[t] = window_function[0] -		// TODO - FIXME?
				window_function[1] * cos(2 * M_PI * t / (fex->conf.winlen - 1.0)) +
				window_function[2] * cos(2 * M_PI * t / (fex->conf.winlen - 1.0)) -
				window_function[3] * cos(2 * M_PI * t / (fex->conf.winlen - 1.0));
	/* loop over signal */
	for (pos = 0, t = 0; pos < fex->wave.samples && t < fex->fft.ntime; pos += fex->conf.hop, ++t) {
		/* copy windowed chunk to in */
		for (i = 0; i < fex->conf.winlen; ++i) {
			if (pos + i < fex->wave.samples)
				in[i][0] = fex->wave.data[pos + i] * window[i];
			else
				in[i][0] = 0.0;
			in[i][1] = 0.0;
		}
		/* calculate fft and fill amp */
		fftw_execute(plan);
		for (f = 0; f < fex->fft.nfreq; ++f)
			fftAmp(fex,t,f) = sqrt(out[f][0] * out[f][0] + out[f][1] * out[f][1]);
	}
	/* zero unused bins */
	for (; t < fex->fft.ntime; ++t) for (f = 0; f < fex->fft.nfreq; ++f)
		fftAmp(fex,t,f) = 0;
	/* drestroy and free data */
	fftw_destroy_plan(plan);
	fftw_free(out);
	fftw_free(in);
	free(window);
	/* psuedo bandpass filter */
	int f_zero, f_count = 0;
	for (f = 0; fex->fft.freq[f] < fex->conf.lopass; ++f);
	f_zero = f;
	for (; fex->fft.freq[f] <= fex->conf.hipass; ++f) f_count++;
	for (f = 0; f < f_count; ++f) fex->fft.freq[f] = fex->fft.freq[f+f_zero];
	fex->fft.freq = realloc(fex->fft.freq, f_count * sizeof(double));

	double *new_amp = malloc(fex->fft.ntime * f_count * sizeof(double));
	for (t = 0; t < fex->fft.ntime; ++t) for (f = 0; f < f_count; ++f)
		new_amp[f_count * t + f] = fex->fft.amp[fex->fft.nfreq * t + f + f_zero];
	fex->fft.nfreq = f_count;
	free(fex->fft.amp);
	fex->fft.amp = new_amp;

	/* normalize, log transform and scale to dB */
	double max = 0.0;
	for (t = 0; t < fex->fft.ntime; ++t) for (f = 0; f < fex->fft.nfreq; ++f)
		if (fftAmp(fex,t,f) > max) max = fftAmp(fex,t,f);
	for (t = 0; t < fex->fft.ntime; ++t) for (f = 0; f < fex->fft.nfreq; ++f)
		fftAmp(fex,t,f) = 10.0 * log10(fftAmp(fex,t,f) / max);
}


fex_t destroy_fft(FEX *fex) {
	if (!fex) return FexNullAccess;
	fex_t ret = FexSuccess;
	if (!fex->fft.amp) ret = FexNullMember;
	free(fex->fft.amp);
	fex->fft.amp = NULL;
	if (!fex->fft.time) ret = FexNullMember;
	free(fex->fft.time);
	fex->fft.time = NULL;
	if (!fex->fft.freq) ret = FexNullMember;
	free(fex->fft.freq);
	fex->fft.freq = NULL;
	return ret;
}

