PROG     =  fex
VER      =  2.0
CC       ?= gcc
DEPS     =  x11 cairo fftw3 sndfile
DEFS		=  -DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}
CFLAGS   += $(shell pkg-config --cflags ${DEPS}) ${DEFS}
LDLIBS   += $(shell pkg-config --libs ${DEPS}) -lm -lXpm
PREFIX   ?= /usr
MODULES  =  config fex fft spectro wave xlib
HEADERS  =  fex.h
MANPAGES =  fex.1 fex-help.1
VPATH    =  src:doc

${PROG}: ${MODULES:%=%.o}

xlib.o: xlib.c xlib_toolwin.c ${HEADERS}

config.o: config.c config.h ${HEADERS}

%.o: %.c ${HEADERS}

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	@install -Dm755 src/${PROG}-gtk ${DESTDIR}${PREFIX}/bin/${PROG}-gtk
	@install -Dm644 doc/${PROG}.1 ${DESTDIR}${PREFIX}/share/man/man1/${PROG}.1
	@install -Dm644 doc/${PROG}-help.1 ${DESTDIR}${PREFIX}/share/man/man1/${PROG}-help.1
	@mkdir -p ${DESTDIR}${PREFIX}/share/${PROG}
	@install -Dm644 share/config ${DESTDIR}${PREFIX}/share/${PROG}/config
	@install -Dm644 share/icon.png ${DESTDIR}${PREFIX}/share/pixmaps/${PROG}.png
	@install -Dm644 share/${PROG}.desktop ${DESTDIR}${PREFIX}/share/applications/${PROG}.desktop

${MANPAGES}: fex%.1: fex%-1.tex
	@latex2man $< doc/$@

man: ${MANPAGES}

clean:
	@rm -f ${MODULES:%=%.o}

distclean: clean
	@rm -f ${PROG} ${PROG}-${VER}.tar.gz

moreclean: distclean
	@cd doc && rm -f ${MANPAGES}

dist: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist distclean man
