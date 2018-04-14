CC = gcc
CXX = g++

SRCS = fbdraw.c

INCLUDEDIR = -I../input
LIBDIR = -L../input
LIBS =

CFLAGS = -fpic -Wall -Werror -O2 $(INCLUDEDIR)
LDFLAGS = -shared $(LIBDIR)
LIBNAME = fbdraw.so

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
