
PROG	=	fex
VER		=	0.2a
SOURCE	=	fex.c xcairo.c
PREFIX	?=	/usr
CFLAGS	+=	-Os -Wall -Wno-unused-parameter -Wno-unused-result
CFLAGS	+=	`pkg-config --cflags sndfile fftw3 x11 cairo`
LDFLAGS	+=	`pkg-config --libs sndfile fftw3 x11 cairo`
DEFS	=	-DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}

${PROG}: ${SOURCE}
ifeq ($(uname),OSX)
	@echo -e "\033[32;1m==>\033[0m Installing dependencies via macports"
	@sudo port install xorg-libX11 libsndfile cairo fftw-3 pkconfig xorg-fonts xinit
endif
	@gcc -o ${PROG} ${DEFS} ${SOURCE} ${CFLAGS} ${LDFLAGS}
	@strip ${PROG}

clean:
	@rm -f ${PROG}

tarball: clean
	@rm -f ${PROG}-${VER}.tar.gz
	@tar -czf ${PROG}-${VER}.tar.gz *

install: ${PROG}
	@echo -en "\033[32;1m==>\033[31m WARNING:\033[0m "
	@echo -e  "alpha versions should not be installed"
	@#install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}


