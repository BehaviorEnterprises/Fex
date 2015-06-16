

PROG      =  glFex
VER       =  3.0a
CC       ?=  gcc

ifeq ($(OS),win)
# fftw3 and sndfile??
DEFS     += -D_WIN32_
CFLAGS   += -I"C:\Program Files\Common Files\MinGW\GLUT\include"
LDLIBS   += -L"C:\Program Files\Common Files\MinGW\GLUT\lib" -lglut32 -lopengl32 -Wl,--subsystem,windows
endif
ifeq ($(OS),linux)
DEPS      =  fftw3 freeglut gl sndfile
CFLAGS   +=  $(shell pkg-config --cflags ${DEPS})
LDLIBS   +=  $(shell pkg-config --libs ${DEPS}) -lm
PREFIX   ?=  /usr
endif

MODULES   =  config fex fft wave
HEADERS   =  fex.h fex_struct.h
MANPAGES  =  fex.1
VPATH     =  src:doc

${PROG}: ${MODULES:%=%.o}

glFex.o: glFex.c fex.h

%.o: %.c ${HEADERS}

clean:
	@rm -f *.o

distclean: clean
	@rm -f glFex glFex.exe

.PHONY: clean distclean
