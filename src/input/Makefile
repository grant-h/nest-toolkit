CC = gcc
CXX = g++

SRCS = nestinput.c

CFLAGS = -fpic -Wall -Werror -O2
LDFLAGS = -shared
LIBNAME = nestinput.so

OBJS = $(SRCS:.c=.o)

ifdef CROSS_COMPILE
CC := $(CROSS_COMPILE)$(CC)
CXX := $(CROSS_COMPILE)$(CC)
endif

all:	$(OBJS)
	$(CC) $(LDFLAGS) -o lib$(LIBNAME) $(OBJS)

clean:
	-rm -rf *.o *.so

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
