
#ifndef __FEX_STRUCT_H__
#define __FEX_STRUCT_H__

#define fftAmp(fex,t,f)		(fex)->fft.amp[(fex)->fft.nfreq * t + f]

typedef struct FEX {
	struct {
		const char *name;
		int samples, rate;
		double *data;
	} wave;
	struct {
		double *time, *freq, *amp;
		int ntime, nfreq;
		int *path;
	} fft;
	struct {
		int winlen, hop;
		double lopass, hipass, floor, threshold;
		// colors
	} conf;
} FEX;

#endif /* __FEX_STRUCT_H__ */
