
PROG	=	fex
VER		=	0.3a
SOURCE	=	fex.c
HEADERS =	fex.h
PREFIX	?=	/usr
CC		?=	gcc
CFLAGS	+=	-Os `pkg-config --cflags sndfile fftw3`
LDFLAGS	+=	`pkg-config --libs sndfile fftw3`
DEFS	=	-DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}

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
SOURCE	+=	wingdi.c
endif
endif
endif

default:
	@echo "please specifiy one of the following targets:"
	@echo -e "  make linux\n  make mac\n  make win"	

${PROG}: ${SOURCE} ${HEADERS}
	@${CC} -o ${PROG} ${DEFS} ${SOURCE} ${CFLAGS} ${LDFLAGS}
	@strip ${PROG}

linux:
	@TARGET=linux make ${PROG} >/dev/null

mac:
	@TARGET=mac make ${PROG} >/dev/null

win:
	@TARGET=win make ${PROG} >/dev/null

clean:
	@rm -f ${PROG}

tarball: clean
	@rm -f ${PROG}-*.tar.gz
	@tar -czf ${PROG}-${VER}.tar.gz *

install: ${PROG}
	@echo -en "\033[32;1m==>\033[31m WARNING:\033[0m "
	@echo -e  "alpha versions should not be installed"
	@#install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}

