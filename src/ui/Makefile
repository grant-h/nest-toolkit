CC = gcc
CXX = g++

SRCS = toolkit.c render.c fonthandler.c

INCLUDEDIR = -I/home/zeus/src/pkgbuilds/arm-freetype2/pkg/freetype2/usr/arm-none-linux-gnueabi/include/freetype2
INCLUDEDIR := -I../input -I../fb $(INCLUDEDIR)
LIBDIR = -L../input -L/home/zeus/src/nest/nestfs/lib -L../fb
LIBS = -lnestinput -lfreetype -lfbdraw

CFLAGS = -fpic -Wall -Werror -O2 $(INCLUDEDIR)
LDFLAGS = -shared $(LIBDIR)
LIBNAME = uitoolkit.so

OBJS = $(SRCS:.c=.o)

ifdef CROSS_COMPILE
CC := $(CROSS_COMPILE)$(CC)
CXX := $(CROSS_COMPILE)$(CC)
endif

all:	$(OBJS)
	$(CC) $(LDFLAGS) -o lib$(LIBNAME) $(OBJS) $(LIBS)

clean:
	-rm -rf *.o *.so

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
