
#include "fft.hpp"

// TODO configurable window_function
static double window_function[4] = { 0.5, 0.5, 0, 0 }; // hanning
/*
Hamming      0.54      0.46      0.00      0.00
Hanning      0.50      0.50      0.00      0.00
Blackman     0.42659   0.49656   0.076849  0.00
Nuttall      0.355768  0.487396  0.144232  0.012604
BlackNutt    0.3635819 0.4891775 0.1365995 0.0106411
BlackHarris  0.35875   0.48829   0.14128   0.01168

*/

Fft::Fft(int argc, const char **argv) : Config(argc, argv) {
	song.loadFromFile(fname);
	ntime = song.getSampleCount() / conf.hop;
	nfreq = conf.winlen / 2 + 0.5;
	time = (double *) malloc(ntime * sizeof(double));
	freq = (double *) malloc(nfreq * sizeof(double));
	amp = (double *) malloc(nfreq * ntime * sizeof(double));
	erase = (unsigned short int *) calloc(nfreq * ntime, sizeof(unsigned short int *));
	/* fill time and freq arrays */
	double nyquist = (double) song.getSampleRate() / 2000.0;
	double dt = song.getDuration().asSeconds() / (double) ntime;
	double df = nyquist / (double) nfreq;
	int i;
	time[0] = freq[0] = 0.0;
	for (i = 1; i < ntime; ++i) time[i] = time[i-1] + dt;
	for (i = 1; i < nfreq; ++i) freq[i] = freq[i-1] + df;
	/* preare fftw */
	fftw_complex *in, *out;
	fftw_plan plan;
	in = (fftw_complex *) fftw_malloc(conf.winlen * sizeof(fftw_complex));
	out = (fftw_complex *) fftw_malloc(conf.winlen * sizeof(fftw_complex));
	plan = fftw_plan_dft_1d(conf.winlen, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	/* windowing function */
	double *window = (double *) malloc(conf.winlen * sizeof(double));
	int t, f, pos;
	for (t = 0; t < conf.winlen; ++t)
		window[t] = window_function[0] -
				window_function[1] * cos(2 * M_PI * t / (conf.winlen - 1.0)) +
				window_function[2] * cos(4 * M_PI * t / (conf.winlen - 1.0)) -
				window_function[3] * cos(6 * M_PI * t / (conf.winlen - 1.0));
	/* loop over signal */
	const sf::Int16 *data = song.getSamples();
	for (pos = 0, t = 0; pos < song.getSampleCount() && t < ntime; pos += conf.hop, ++t) {
		/* copy windowed chunk to in */
		for (i = 0; i < conf.winlen; ++i) {
			if (pos + i < song.getSampleCount())
				in[i][0] = data[pos + i] * window[i];
			else
				in[i][0] = 0.0;
			in[i][1] = 0.0;
		}
		/* calculate fft and fill amp */
		fftw_execute(plan);
		for (f = 0; f < nfreq; ++f)
			amp[nfreq * t + f] = sqrt(out[f][0] * out[f][0] + out[f][1] * out[f][1]);
	}
	/* zero unused bins */
	for (; t < ntime; ++t) for (f = 0; f < nfreq; ++f)
		amp[nfreq * t + f] = 0;
	/* drestroy and free data */
	fftw_destroy_plan(plan);
	fftw_free(out);
	fftw_free(in);
	free(window);
	/* psuedo bandpass filter */
	int f_zero, f_count = 0;
	for (f = 0; freq[f] < conf.lopass; ++f);
	f_zero = f;
	for (; freq[f] <= conf.hipass; ++f) f_count++;
	for (f = 0; f < f_count; ++f) freq[f] = freq[f+f_zero];
	freq = (double *) realloc(freq, f_count * sizeof(double));
	double *new_amp = (double *) malloc(ntime * f_count * sizeof(double));
	for (t = 0; t < ntime; ++t) for (f = 0; f < f_count; ++f)
		new_amp[f_count * t + f] = amp[nfreq * t + f + f_zero];
	nfreq = f_count;
	free(amp);
	amp = new_amp;
	/* normalize, log transform and scale to dB */
	double max = 0.0;
	for (t = 0; t < ntime; ++t) for (f = 0; f < nfreq; ++f)
		if (amp[nfreq * t + f] > max) max = amp[nfreq * t + f];
	for (t = 0; t < ntime; ++t) for (f = 0; f < nfreq; ++f)
		amp[nfreq * t + f] = 10.0 * log10(amp[nfreq * t + f] / max);
	lines.setPrimitiveType(sf::LinesStrip);
	lines.resize(ntime);
	points.setPrimitiveType(sf::Quads);
	points.resize(ntime * 4);
	makeSpectrogram();
	makeThreshold();
	makeOverlay();
}

void Fft::makeSpectrogram() {
	int t, f;
	sf::Image img;
	img.create(ntime, nfreq);
	double dd;
	unsigned short int di;
	for (f = 0; f < nfreq; ++f) for (t = 0; t < ntime; ++t) {
		dd = 255 * (1 + amp[nfreq * t + f] / conf.floor);
		di = (dd > 255 ? 255 : (dd < 0 ? 0 : dd));
		img.setPixel(t, f, sf::Color(255, 255, 255, di));
	}
	texSpec.loadFromImage(img);
	texSpec.setSmooth(true);
	spec = sf::Sprite(texSpec);
	spec.setScale(1.0,-1.0);
}

void Fft::makeOverlay() {
// TODO add fex calculation here?
//   * Change function name to "recalculate"?
	int t, f, fmax, n;;
	double max;
	for (t = 0, n = 0; t < ntime; ++t) {
		fmax = -1;
		max = std::numeric_limits<double>::lowest(); // TODO: is this negative?  Should it be?
		for (f = 0; f < nfreq; ++f) {
			if (erase[nfreq * t + f]) continue;
			if (amp[nfreq * t + f] < max) continue;
			max = amp[nfreq * t + (fmax=f)];
		}
		if (max < -1.0 * conf.threshold) continue;
		lines[n].position = sf::Vector2f(t + 0.5, - fmax - 0.5);
		points[4*n].position = sf::Vector2f(t, - fmax);
		points[4*n+1].position = sf::Vector2f(t + 1, - fmax);
		points[4*n+2].position = sf::Vector2f(t + 1, - fmax - 1);
		points[4*n+3].position = sf::Vector2f(t, - fmax - 1);
// TODO: These next lines should only need to be done once ... move to constructor?
		lines[n].color = conf.linesFG;
		points[4*n].texCoords = sf::Vector2f(0,0);
		points[4*n+1].texCoords = sf::Vector2f(64,0);
		points[4*n+2].texCoords = sf::Vector2f(64,64);
		points[4*n+3].texCoords = sf::Vector2f(0,64);
		++n;
	}
	for (; n < ntime; ++n) {
		lines[n].position = lines[n-1].position;
		points[4*n].position = sf::Vector2f(-2,0);
		points[4*n+1].position = sf::Vector2f(-2,0);
		points[4*n+2].position = sf::Vector2f(-2,0);
		points[4*n+3].position = sf::Vector2f(-2,0);
	}
}

void Fft::makeThreshold() {
	int t, f;
	sf::Image img;
	img.create(ntime, nfreq);
	for (f = 0; f < nfreq; ++f) for (t = 0; t < ntime; ++t)
		img.setPixel(t, f, sf::Color(255, 255, 255, (amp[nfreq * t + f] > - conf.threshold ? 255 : 0)));
	texThresh.loadFromImage(img);
	texThresh.setSmooth(true);
	thresh = sf::Sprite(texThresh);
	thresh.setScale(1.0,-1.0);
}

void Fft::eraseShift() {
	int t, f;
	unsigned short int *a;
	for (f = 0; f < nfreq; ++f) for (t = 0; t < ntime; ++t) {
		a = &erase[nfreq * t + f];
		if (*a) *a |= (*a<<1);
	}
}

void Fft::eraseUndo() {
	int t, f;
	unsigned short int *a;
	for (f = 0; f < nfreq; ++f) for (t = 0; t < ntime; ++t) {
		a = &erase[nfreq * t + f];
		if (!(*a & 0xFF)) *a = (*a>>1);
	}
}

void Fft::erasePoint(int x, int y) {
	if (x < ntime && y < nfreq) erase[nfreq * x + y] |= 0x01;
}

Fft::~Fft() {
	if (amp) free(amp); amp = NULL;
	if (time) free(time); time = NULL;
	if (freq) free(freq); freq = NULL;
	if (erase) free(erase); erase = NULL;
	ntime = nfreq = 0;
}

