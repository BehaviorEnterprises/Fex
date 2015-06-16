/**********************************************************************\
* WAVE.C
* Part of FEX, see FEX.C for complete license information
* Author: Jesse McClure, copyright 2013-2015
* License; GPL3
\**********************************************************************/

#include <sndfile.h>
#include "fex.h"
#include "fex_struct.h"

fex_t read_wave(FEX *fex) {
	if (!fex) return FexNullAccess;
	SF_INFO info;
	SNDFILE *file = sf_open(fex->wave.name, SFM_READ, &info);
	if (!file) return FexWaveOpen;
	/* read in if single channel */
	fex->wave.samples = info.frames;
	fex->wave.rate = info.samplerate;
	fex->wave.data = malloc(fex->wave.samples * sizeof(double));
	sf_count_t n;
	if (info.channels == 1) {
		n = sf_readf_double(file, fex->wave.data, fex->wave.samples);
	}
	/* otherwise downmix to single channel */
	else {
		double *multi = malloc(info.frames * info.channels * sizeof(double));
		n = sf_readf_double(file, multi, info.frames * info.channels);
		/* average across all channels */
		int i, nframe;
		double mix;
		for (nframe = 0; nframe < info.frames; ++nframe) {
			mix = 0.0;
			for (i = 0; i < info.channels; ++i)
				mix += multi[nframe * info.channels + i];
			fex->wave.data[nframe] = mix / info.channels;
		}
		free(multi);
	}
	/* close file and check that we read the right number of samples */
	sf_close(file);
	if (n != fex->wave.samples)
		fprintf(stderr, "[Fex:Wave] Warning reading \"%s\"\n\tRead %d / %d samples\n",
				fex->wave.name, n, fex->wave.samples);
	return FexSuccess;
}

fex_t free_wave(FEX *fex) {
	if (!fex) return FexNullAccess;
	if (!fex->wave.data) return FexNullMember;
	free(fex->wave.data);
	fex->wave.data = NULL;
	return FexSuccess;
}
