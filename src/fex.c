
#include "fex.h"

int die(const char *msg) {
	fprintf(stderr,"ERROR: %s\n",msg);
	exit(1);
}



int main(int argc, const char **argv) {
	const char *fname = configure(argc,argv);
	Wave *wav = create_wave(fname);
	FFT *fft = create_fft(wav);
	create_spectro(fft, fname);
	free_wave(&wav);

	xlib_event_loop();

	free_spectro();
	free_fft(&fft);
	return 0;
}
