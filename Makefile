
PROG     =  fex
VER      =  2.0a
CC       ?= gcc
CFLAGS   += `pkg-config --cflags x11 cairo freetype2 fftw3 sndfile`
LDFLAGS  += `pkg-config --libs x11 cairo freetype2 fftw3 sndfile` -lm -lXpm
PREFIX   ?= /usr
MODULES  =  config fex fft spectro wave xlib
HEADERS  =  fex.h
MANPAGES =  fex.1 fex-help.1
DEFS		=  -DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}
VPATH    =  src:doc

${PROG}: ${MODULES:%=%.o}
	@echo -e "\033[1;34m  ->\033[0m Linking $@"
	@cd src && ${CC} -o ../$@ ${MODULES:%=%.o} ${LDFLAGS}

xlib.o: xlib.c xlib_toolwin.c ${HEADERS}
	@echo -e "\033[1;34m  ->\033[0m Building $@"
	@${CC} -c -o src/$@ $< ${CFLAGS} ${DEFS}

config.o: config.c config.h ${HEADERS}
	@echo -e "\033[1;34m  ->\033[0m Building $@"
	@${CC} -c -o src/$@ $< ${CFLAGS} ${DEFS}

%.o: %.c ${HEADERS}
	@echo -e "\033[1;34m  ->\033[0m Building $@"
	@${CC} -c -o src/$@ $< ${CFLAGS} ${DEFS}

install: ${PROG} ${MANPAGES}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	@install -Dm755 src/${PROG}-gtk ${DESTDIR}${PREFIX}/bin/${PROG}-gtk
	@install -Dm644 doc/${PROG}.1 ${DESTDIR}${PREFIX}/share/man/man1/${PROG}.1
	@install -Dm644 doc/${PROG}-help.1 ${DESTDIR}${PREFIX}/share/man/man1/${PROG}-help.1
	@install -Dm644 share/config ${DESTDIR}${PREFIX}/share/${PROG}/config
	@install -Dm644 share/icon.png ${DESTDIR}${PREFIX}/share/pixmaps/${PROG}.png
	@install -Dm644 share/${PROG}.desktop ${DESTDIR}${PREFIX}/share/applications/${PROG}.desktop

${MANPAGES}: fex%.1: fex%-1.tex
	@echo -e "\033[1;34m  ->\033[0m Building $@"
	@latex2man $< doc/$@

man: ${MANPAGES}

clean:
	@rm -f ${PROG} ${PROG}-${VER}.tar.gz
	@cd src && rm -f ${MODULES:%=%.o}
	@cd doc && rm -f ${MANPAGES}

dist: clean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist man
