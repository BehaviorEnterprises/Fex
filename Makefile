
PROG      =  glFex
VER       =  3.0a
CC       ?=  gcc
MODULES   =  config fex fft wave
HEADERS   =  fex.h fex_struct.h
MANPAGES  =  fex.1
VPATH     =  src:doc


## build rules:

${PROG}: ${MODULES:%=%.o}

glFex.o: glFex.c fex.h

%.o: %.c ${HEADERS}

clean:
	@rm -f *.o

distclean: clean
	@rm -f glFex glFex.exe

## platform targets, linux is the default:

linux:
	@${MAKE}

#win32:
#	@OS=win32 ${MAKE}

win64:
	@OS=win64 ${MAKE}

.PHONY: clean distclean win32 win64 linux

## platform-specific variable setting:
# Note: the .pc files for freeglut and sndfile don't seem to work for static
# linking.  I had to modify them replacing -lglut with -lfreeglut_static in the
# former, and adding -lvorbis and -logg to the latter while also sorting the lib
# dependencies for sndfile appropriately (as order matters).
ifeq ($(OS),win64)
CC        = x86_64-w64-mingw32-gcc
DEPS      = sndfile fftw3 freeglut
CFLAGS   += -DWIN64_BUILD -DFREEGLUT_STATIC $(shell x86_64-w64-mingw32-pkg-config --static --cflags ${DEPS})
LDFLAGS  += -static
LDLIBS    = $(shell x86_64-w64-mingw32-pkg-config --static --libs ${DEPS})
#else ifeq ($(OS),win32)
else # default = linux
DEPS      =  fftw3 freeglut gl sndfile
CFLAGS   +=  $(shell pkg-config --cflags ${DEPS})
LDLIBS   +=  $(shell pkg-config --libs ${DEPS}) -lm
PREFIX   ?=  /usr
endif

