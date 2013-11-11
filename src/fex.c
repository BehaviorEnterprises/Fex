/*************************************************************************\
* FEX - Frequency excursion estimator
*
* Author: Jesse McClure, copyright 2013
* License: GPLv3
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
\*************************************************************************/


#include "fex.h"

static FFT *create_fft(Wave *,int, int);
static const char *command_line(int, const char **);
static void free_fft(FFT *);
static void free_wave(Wave *);
static Wave *read_wave(const char *);

//static double hipass = 1.25, lopass = 10.0;
static int winlen = 0, hop = 0, interactive = 1;
static SampleWindow custom;
static const SampleWindow *wn = NULL;
static const SampleWindow windows[] = {
	{ "hanning", 		{0.5,			0.5,			0.0,			0.0}			},
	{ "hamming",	 	{0.54,		0.46,			0.0,			0.0}	 		},
	{ "blackman",		{0.42659,	0.49656,		0.076849,	0.0}			},
	{ "nuttall",		{0.355768,	0.487396,	0.144232,	0.012604}	},
	{ "blacknut",		{0.3635819,	0.4891775,	0.1365995,	0.0106411}	},
	{ "blackharris",	{0.35875,	0.48829,		0.14128,		0.01168}		},
	{ "rectangular",	{1.0,			0.0,			0.0,			0.0}			},
};

#define STRING(s)		STRINGIFY(s)
#define STRINGIFY(s)	#s

static inline void version() {
	printf(STRING(PROGRAM_NAME) " v" STRING(PROGRAM_VER)
", Copyright © 2013 Jesse McClure <www.mccluresk9.com>\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
	 exit(0);
}

static inline void help() {
	printf("\n"
"USAGE\n  " STRING(PROGRAM_NAME) " <options> <wavefile>\n\n"
"OPTIONS\n"
"  -v                show version information then exit\n"
"  -h                show help information then exit\n"
"  -i                inhibit interactive plotting mode\n"
"  -w <name>         select a windowing function (default=hanning)\n"
"  -l <length>       dft window length in number of samples (default=256)\n"
"  -o <offset>       offset between windows (default=window length)\n"
"  -t <threshold>    minimum amplitude, in dB below signal max, to accept for peaks (default=24.0)\n\n"
"WINDOW FUNCTIONS\n"
"  Any function that can be represented as a 4-term cosine function can be\n"
"  used for windowing.  Choose from any of the preprogrammed options below,\n"
"  or use \"custom=a0,a1,a2,a3\" to define your own.\n\n"
"  rectangular       rectangular (1.0,0.0,0.0,0.0)\n"
"  hamming           Hamming (0.54,0.46,0,0).\n" 
"  hanning           Hann (0.5,0.5,0.0,0.0) [default]\n"
"  blackman          Blackman (0.42659,0.49656,0.076849,0.0)\n"
"  nuttall           Nutall (0.355768,0.487396,0.144232,0.012604)\n"
"  blacknut          Blackman-Nuttall (0.3635819,0.4891775,0.1365995,0.0106411)\n"
"  blackharris       Blackman-Harris (0.35875,0.48829,0.14128,0.01168)\n"
"  custom=...        select alpha values for a custom function\n\n"
	); exit(0);
}

FFT *create_fft(Wave *w, int win, int hop) {
	FFT *fft = (FFT *) calloc(1,sizeof(FFT));
	/* create amp matrix + freq array.  Calculate freqs */
	fft->fs = win/2 + 1;
	fft->ts = w->samples/hop;
	fft->amp = (double **) malloc(fft->ts * sizeof(double *));
	fft->freq = (double *) malloc(fft->fs * sizeof(double));
	fft->time = (double *) malloc(fft->ts * sizeof(double));
	double nf = (double)w->rate/2000.0;
	double df = nf/fft->fs;
	double dt = (double)w->samples / (double)(w->rate * fft->ts);
	double f,t;
	int i,j, pos;
	for (i = 0, f = 0.0; i < fft->fs; i++, f+=df) fft->freq[i] = f;
	for (i = 0, t = 0.0; i < fft->ts; i++, t+=dt) fft->time[i] = t;
	/* prepare fftw */
	fftw_complex *in, *out;
	fftw_plan p;;
	in = (fftw_complex *) fftw_malloc(win * sizeof(fftw_complex));
	out = (fftw_complex *) fftw_malloc(win * sizeof(fftw_complex));
	p = fftw_plan_dft_1d(win, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	/* create Hamming window */
	double window[win];
	for (i = 0; i < win; i++)
		window[i] = wn->a[0] - wn->a[1] * cos(2*M_PI*(i/((win-1)*1.0))) +
				wn->a[2] * cos( 2 * M_PI * (i / ((win - 1) * 1.0))) -
				wn->a[3] * cos( 2 * M_PI * (i / ((win - 1) * 1.0)));
	/* loop over signal */
	for (pos = 0, j = 0; pos < w->samples; pos += hop, j++) {
		fft->amp[j] = (double *) malloc(fft->fs * sizeof(double));
		/* copy windowed chunk to dat */
		for (i = 0; i < win; i++) {
			if (pos+i < w->samples) {
				in[i][0]=w->d[pos+i]*window[i]; in[i][1]=0.0;
			}
			else {
				in[i][0] = 0.0; in[i][1] = 0.0;
				goto done; /* break out two levels */
			}
		}
		/* calculate fft */
		fftw_execute(p);
		/* fill amp matrix */
		for (i = 0; i < fft->fs; i++)
			fft->amp[j][i] = sqrtf(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
	}
	done:
	fftw_destroy_plan(p);
	fftw_free(out);
	fftw_free(in);
	/* fill and zero unused bins */
	for ( ; j < fft->ts; j++)
		fft->amp[j] = (double *) calloc(fft->fs, sizeof(double));
	/* band pass filter */
	for (i = 0; i < fft->ts; i++) {
		for (j = 0; j < fft->fs && fft->freq[j] < hipass; j++)
			fft->amp[i][j] = 0;
		for (j = fft->fs-1; j && fft->freq[j] > lopass; j--)
			fft->amp[i][j] = 0;
	}
	/* normalize, log transform amplitude, and scale to dB */
	max = 0.0, min = 0.0;
	for (i = 0; i < fft->ts; i++) for (j = 0; j < fft->fs; j++)
		if (fft->amp[i][j] > max) max = fft->amp[i][j];
	for (i = 0; i < fft->ts; i++) for (f = 0, j = 0; j < fft->fs; j++) {
		fft->amp[i][j] = 10.0 * log10(fft->amp[i][j]/max);
		if (fft->amp[i][j]<min && fft->amp[i][j]>-900) min = fft->amp[i][j];
	}
	max = 0.0;
//	for (i = 0; i < fft->ts; i++) for (f = 0, j = 0; j < fft->fs; j++)
//		if (fft->amp[i][j] > max) max = fft->amp[i][j];
	return fft;
}

const char *command_line(int argc, const char **argv) {
	const char *winfun = NULL, *wavname = NULL;
	char a; int i;
	thresh = -14.0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			a = (argv[i][1] == '-' ? argv[i][2] : argv[i][1]);
			if (a == 'w' && argc > (++i))
				winfun = argv[i];
			else if (a == 'l' && argc > (++i))
				winlen = atoi(argv[i]);
			else if (a == 't' && argc > (++i))
				thresh = -1.0 * atof(argv[i]);
			else if (a == 'o' && argc > (++i))
				hop = atof(argv[i]);
			else if (a == 'i')
				interactive = 0;
			else if (a == 'v')
				version();
			else if (a == 'h')
				help();
			else
				fprintf(stderr,"ignoring unrecognized option \"%s\"\n",
						argv[i]);
		}
		else if (!wavname) {
			wavname = argv[i];
		}
		else {
			fprintf(stderr,"ignoring unrecognized parameter \"%s\"\n",
					argv[i]);
		}
	}
	if (winfun) {
		if (strncmp(winfun,"custom",6)==0 &&
				sscanf(winfun,"custom=%lf,%lf,%lf,%lf",
				&custom.a[0],&custom.a[1], &custom.a[2], &custom.a[3]) == 4)
			wn = &custom;
		else
			for (i = 0; i < sizeof(windows)/sizeof(windows[0]); i++)
				if (strncmp(windows[i].type,winfun,strlen(winfun))==0)
					wn = &windows[i];
	}
	if (!wn) wn = windows;
	if (!wavname) die("no wave file provided");
	name = strrchr(wavname,'/');
	name = name ? name + 1 : wavname;
	return wavname;
}

int die(const char *msg) {
	fprintf(stderr,"ERROR: %s\n",msg);
	exit(1);
}

void free_fft(FFT *fft) {
	int i;
	for (i = 0; i < fft->ts; i++) free(fft->amp[i]);
	/* why does this double-free fault? */
//	free(fft->amp);
	free(fft->time);
	free(fft->freq);
	free(fft);
}

void free_wave(Wave *w) {
	free(w->d);
	free(w);
}

Wave *read_wave(const char *fname) {
	Wave *w = (Wave *) malloc(sizeof(Wave));
	SF_INFO *wi; SNDFILE *wf = NULL;
	wi = (SF_INFO *) malloc(sizeof(SF_INFO));
	wf = sf_open(fname,SFM_READ,wi);
	w->samples = wi->frames;
	w->rate = wi->samplerate;
	if (range[1]) {
		w->samples = (range[1]-range[0] + 1)*hop;
		sf_seek(wf,range[0]*hop,SEEK_SET);
	}
	w->d = (double *) malloc(w->samples*sizeof(double));
	sf_count_t n = sf_readf_double(wf,w->d,w->samples);
	if (n != w->samples) printf("error\n");
	sf_close(wf);
	if (!winlen) winlen = 256;
	if (!hop) hop = winlen / 4;
	return w;
}

int main(int argc, const char **argv) {
	int restart = 1, previewing, i,j, f;
	sp_floor = -30.0;
	memset(range,0,sizeof(range));
	hipass = 1.25; lopass = 15.0;
	double lt, lf;
	long double ex, tex;
	if (interactive) preview_init();
	while (restart) {
		restart = 1;
		Wave *w = read_wave(command_line(argc,argv));
		while (restart == 1) {
			FFT *fft = create_fft(w,winlen,hop);
			if (interactive) preview_create(fft->ts,fft->fs,fft);
			previewing = 1;
			ex = tex = 0.0;
			while (previewing) {
				lt = fft->time[0]; lf = fft->freq[0];
				ex = 0.0; tex = 0.0;
				preview_threshold_start();
				for (i = 1; i < fft->ts; i++)
					for (j = 0; j < fft->fs; j++)
						if (fft->amp[i][j] > thresh) preview_threshold(i,j);
				preview_peak_start();
				for (i = 1; i < fft->ts; i++) {
					for (f = 0, j = 0; j < fft->fs; j++)
						if ( (fft->amp[i][j] > fft->amp[i][f] || !f) )
							f = j;
					if (f > 0 && fft->amp[i][f] > thresh ) {
						if (lt != fft->time[0]) {
							ex += sqrtf((fft->freq[f] - lf) *
									(fft->freq[f] - lf) + (fft->time[i] - lt) *
									(fft->time[i] - lt));
							tex += fft->time[i] - lt;
						}
						lt = fft->time[i]; lf = fft->freq[f];
						if (interactive) preview_peak(i,f);
					}
				}
				if (interactive) previewing = preview_test(ex,tex);
				else previewing = 0;
			}
			free_fft(fft);
			if (interactive) restart = preview_destroy();
			else restart = 0;
		}
		free_wave(w);
	}
	if (interactive) preview_end();
	printf("%Lf\n",ex/tex);
	return 0;
}

