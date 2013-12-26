
#include "fex.h"

Wave *create_wave(const char *fname) {
	Wave *w = (Wave *) calloc(1,sizeof(Wave));
	SF_INFO *winfo;
	SNDFILE *wfile = NULL;
	winfo = (SF_INFO *) calloc(1, sizeof(SF_INFO));
	wfile = sf_open(fname, SFM_READ, winfo);
	w->samples = winfo->frames;
	w->rate = winfo->samplerate;
	w->d = (double *) calloc(w->samples, sizeof(double));
	sf_count_t n = sf_readf_double(wfile, w->d, w->samples);
	if (n != w->samples)
		fprintf(stderr,"Error reading \"%s\"\n\t"
				"\t%d samples read of\n\t%d reported size\n",
				fname, n, w->samples);
	sf_close(wfile);
	return w;
}

int free_wave(Wave **wp) {
	Wave *w = *wp;
	free(w->d);
	free(w);
}
