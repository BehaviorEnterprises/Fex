PROG        =  fex
VER         =  3.0pre
PREFIX     ?=  /usr
CC          =  g++
CONFFILE   ?=  ${PREFIX}/share/${PROG}/${PROG}rc
DEFS        =  -DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER} -DDEFAULT_CONFIG=${CONFFILE}
CPPFLAGS   ?=  -D_FORTIFY_SOURCE=2
CXXFLAGS   ?=  -march=native -O2 -pipe -fstack-protector-strong --param=ssp-buffer-size=4
LDFLAGS    ?=  -Wl,-O1,--sort-common,--as-needed,-z,relro
CXXFLAGS   +=  $(shell pkg-config --cflags sfml-all fftw3) -std=c++11 ${DEFS}
LDLIBS     +=  $(shell pkg-config --libs sfml-all fftw3)
MODULES     =  config fex fft spectrogram
MANPAGES    =  ${PROG}.1 ${PROG}rc.5
VPATH       =  src

${PROG}: ${MODULES:%=%.o}

%.o: %.cpp %.hpp

install: ${PROG}
	@echo ${PROG}-${VER} is not a release candidate and is not suitable for installation

clean:
	@rm -rf ${PROG}-*.tar.gz
	@rm -rf ${MODULES:%=%.o}

distclean: clean
	@rm -rf ${PROG}

tarball: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean distclean install tarball
