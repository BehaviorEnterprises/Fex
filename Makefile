
PROG	=	fex
VER		=	0.2a
TARGET	?=	linux
HEADERS =	fex.h
PREFIX	?=	/usr
DEFS	=	-DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}

ifeq "${TARGET}" "linux"
SOURCE	=	fex.c xcairo.c
CFLAGS	+=	-Os -Wall -Wno-unused-parameter -Wno-unused-result
CFLAGS	+=	`pkg-config --cflags sndfile fftw3 x11 cairo`
LDFLAGS	+=	`pkg-config --libs sndfile fftw3 x11 cairo`
else
ifeq "${TARGET}" "mac"
SOURCE	=	fex.c osxcg.m
CFLAGS	+=	-Os -framework Cocoa
CFLAGS	+=	`pkg-config --cflags sndfile fftw3`
LDFLAGS	+=	`pkg-config --libs sndfile fftw3`
else
ifeq "${TARGET}" "win"
$(error No windows port is available)
endif
endif
endif

${PROG}: ${SOURCE} ${HEADERS}
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

