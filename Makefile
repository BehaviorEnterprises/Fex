
PROG	=	fex
VER		=	0.2a
SOURCE	=	fex.c
HEADERS =	fex.h
PREFIX	?=	/usr
CFLAGS	+=	-Os `pkg-config --cflags sndfile fftw3`
LDFLAGS	+=	`pkg-config libs sndfile fftw3`
DEFS	=	-DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}
TARGET	?=	linux

ifeq "${TARGET}" "linux"
SOURCE	+=	xcairo.c
CFLAGS	+=	-Wall `pkg-config --cflags x11 cairo`
LDFLAGS	+=	`pkg-config --libs x11 cairo`
else
ifeq "${TARGET}" "mac"
SOURCE	+=	osxcg.m
CFLAGS	+=	-framework Cocoa
else
ifeq "${TARGET}" "win"
$(error No windows port is available yet)
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

