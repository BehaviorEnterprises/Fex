
#include "fex.h"

#define FFTW_FLAGS	FFTW_FORWARD, FFTW_ESTIMATE
FFT *create_fft(Wave *wav) {
	/* allocate memory */
	FFT *fft = (FFT *) calloc(1,sizeof(FFT));
	fft->nfreq = conf.winlen/2 + 1;
	fft->ntime = wav->samples/conf.hop;
	fft->amp = (double **) calloc(fft->ntime, sizeof(double *));
	fft->time = (double *) calloc(fft->ntime, sizeof(double));
	fft->freq = (double *) calloc(fft->nfreq, sizeof(double));
	fft->mask = (char **) calloc(fft->ntime, sizeof(char *));
	/* calculate step sizes and fill time/freq arrays */
	double nyquist = (double) wav->rate / 2000.0;
	double df = nyquist / fft->nfreq;
	double dt = (double)wav->samples / (double)(wav->rate * fft->ntime);
	double f, t;
	int i, j;
	for (i = 0, f = 0.0; i < fft->nfreq; i++, f += df) fft->freq[i] = f; 
	for (i = 0, t = 0.0; i < fft->ntime; i++, t += dt) fft->time[i] = t; 
	/* prepare fftw */
	fftw_complex *in, * out;
	fftw_plan p;
	in = (fftw_complex *)fftw_malloc(conf.winlen * sizeof(fftw_complex));
	out = (fftw_complex *)fftw_malloc(conf.winlen * sizeof(fftw_complex));
	p = fftw_plan_dft_1d(conf.winlen, in, out, FFTW_FLAGS);
	/* create windowing function */
	double window[conf.winlen];
	double *a = conf.win->a;
	double wl = conf.winlen;
	for (i = 0; i < conf.winlen; i++)
		window[i] = a[0] - a[1] * cos(2 * M_PI * (i / (wl - 1.0))) +
				a[2] * cos(2 * M_PI * (i / (wl - 1.0))) -
				a[3] * cos(2 * M_PI * (i / (wl - 1.0)));
	/* loop over signal */
	int pos;
	for (pos = 0, j = 0; pos < wav->samples; pos += conf.hop, j++) {
		fft->amp[j] = (double *) malloc(fft->nfreq * sizeof(double));
		fft->mask[j] = (char *) calloc(fft->nfreq, sizeof(char));
		/* copy windowed chunk to dat */
		for (i = 0; i < conf.winlen; i++) {
			if (pos + i < wav->samples) {
				in[i][0] = wav->d[pos + i] * window[i];
				in[i][1] = 0.0;
			}
			else {
				in[i][0] = 0.0;
				in[i][1] = 0.0;
				goto doublebreak;
			}
		}
		/* calculate fft & fill amp matrix */
		fftw_execute(p);
		for (i = 0; i < fft->nfreq; i++)
			fft->amp[j][i] = sqrt(out[i][0] * out[i][0] +
					out[i][1] * out[i][1]);
	}
	doublebreak:
	fftw_destroy_plan(p);
	fftw_free(out);
	fftw_free(in);
	/* fill and zero unused bins */
	for ( ; j < fft->ntime; j++) {
		fft->amp[j] = (double *) calloc(fft->nfreq, sizeof(double));
		fft->mask[j] = (char *) calloc(fft->nfreq, sizeof(char));
	}
	/* band pass filter */
	for (i = 0; i < fft->ntime; i++) {
		for (j = 0; j < fft->nfreq && fft->freq[j] < conf.hipass; j++)
			fft->amp[i][j] = 0;
		for (j = fft->nfreq - 1; fft->freq[j] > conf.lopass; j--)
			fft->amp[i][j] = 0;
	}
	/* normalize, log transform, and scale to dB */
	fft->max = fft->min = 0.0;
	for (i = 0; i < fft->ntime; i++) for (j = 0; j < fft->nfreq; j++)
		if (fft->amp[i][j] > fft->max) fft->max = fft->amp[i][j];
	for (i = 0; i < fft->ntime; i++) for (j = 0; j < fft->nfreq; j++) {
		fft->amp[i][j] = 10.0 * log10(fft->amp[i][j] / fft->max);
		if (fft->amp[i][j] < fft->min && fft->amp[i][j] > -900)
			fft->min = fft->amp[i][j];
	}
	fft->max = 0.0;
	return fft;
}

int free_fft(FFT **fftp) {
	FFT *fft = *fftp;
	int i;
	for (i = 0; i < fft->ntime; i++) {
		free(fft->amp[i]);
		free(fft->mask[i]);
	}
	//free(fft->amp);
	//free(fft->time);
	free(fft->freq);
	free(fft);
	return 0;
}
