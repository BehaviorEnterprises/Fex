
#ifndef __FEX_H__
#define __FEX_H__

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

typedef int fex_t;

enum {
	FexWaveOpen = INT_MIN,
	FexNullMember,
	FexNullAccess,
	FexMallocError,
	FexSuccess = 0,
} FexStatus;

extern fex_t fex_create(const char *);
extern fex_t fex_destroy(fex_t);
extern unsigned char *fex_get_spectrogram(fex_t, int *, int *);
extern int *fex_get_path(fex_t);
extern int fex_threshold(fex_t, float);
extern double fex_measure(fex_t, double *, double*);
extern int fex_coordinates(fex_t, double, double, double *, double*);

#endif /* __FEX_H__ */
