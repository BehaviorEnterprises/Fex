
PROG		=	fex
VER		=	0.3a
SOURCE	=	fex.c
HEADERS	=	fex.h
PREFIX	?=	/usr
CC			?=	gcc
CFLAGS	+=	`pkg-config --cflags sndfile fftw3`
LDFLAGS	+=	`pkg-config --libs sndfile fftw3`
DEFS		=	-DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}

SOURCE	+= xcairo.c
CFLAGS	+= `pkg-config --cflags x11 cairo`
LDFLAGS	+= `pkg-config --libs x11 cairo`
mac_SOURCE		+= osxcg.m
mac_CFLAGS		+= -framework Cocoa
win_SOURCE		+= wingdi.c

${PROG}: ${SOURCE} ${HEADERS}
	@${CC} -o ${PROG} ${DEFS} ${SOURCE} ${CFLAGS} ${LDFLAGS}

clean:
	@rm -f ${PROG}

tarball: clean
	@rm -f ${PROG}-*.tar.gz
	@tar -czf ${PROG}-${VER}.tar.gz *

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}

mac_install: ${PROG}
	@cp ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	@cp -r MacFex.app ${DESTDIR}/Applications/

win_intstall: ${PROG}
	@echo "windows port is not complete"

