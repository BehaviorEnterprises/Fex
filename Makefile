PROG        = fex
VER         = 3.0pre
CC          = g++
CPPFLAGS    = -D_FORTIFY_SOURCE=2
CFLAGS      = -march=native -O2 -pipe -fstack-protector-strong --param=ssp-buffer-size=4
CXXFLAGS    = -march=native -O2 -pipe -fstack-protector-strong --param=ssp-buffer-size=4
LDFLAGS     = -Wl,-O1,--sort-common,--as-needed,-z,relro
CXXFLAGS   += $(shell pkg-config --cflags sfml-all fftw3) -std=c++11
LDLIBS     += $(shell pkg-config --libs sfml-all fftw3)
MODULES     = config fex fft spectrogram
VPATH       = src

${PROG}: ${MODULES:%=%.o}

%.o: %.cpp %.hpp

clean:
	@rm -rf ${PROG}-*.tar.gz
	@rm -rf ${MODULES:%=%.o}

distclean: clean
	@rm -rf ${PROG}

tarball: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean distclean tarball
