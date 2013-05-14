
PROG	=	fex
VER		=	0.2a
SOURCE	=	fex.c xcairo.c
CFLAGS	+=	-Os -Wall
CFLAGS	+=	`pkg-config --cflags sndfile fftw3 x11 cairo`
LDFLAGS	+=	`pkg-config --libs sndfile fftw3 x11 cairo`
DEFS	=	-DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}

${PROG}: ${SOURCE}
	@gcc -o ${PROG} ${DEFS} ${SOURCE} ${CFLAGS} ${LDFLAGS}
	@strip ${PROG}

clean:
	@rm -f ${PROG}

tarball: clean
	@rm -f ${PROG}-${VER}.tar.gz
	@tar -czf ${PROG}-${VER}.tar.gz *

install:
	@echo -en "\033[32;1m ==>\033[31m WARNING:\033[0m "
	@echo -e  "alpha versions should not be installed"

macprep:
	@sudo port install xorg-libX11 libsndfile cairo fftw-3 pkgconfig


