
#include "fex.h"

int die(const char *msg, ...) {
	va_list arg;
	fprintf(stderr,"Fatal Error: ");
	va_start(arg, msg);
	vfprintf(stderr,msg,arg);
	va_end(arg);
	exit(1);
}



int main(int argc, const char **argv) {
	const char *fname = configure(argc,argv);
	Wave *wav = create_wave(fname);
	FFT *fft = create_fft(wav);
	create_spectro(fft, fname);
	free_wave(&wav);

	xlib_event_loop();
	fprintf(stdout,"%.3lf\n", spect->fex);

	free_spectro();
	free_fft(&fft);
	return 0;
}
