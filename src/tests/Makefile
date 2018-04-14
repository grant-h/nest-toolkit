CC = gcc
CXX = g++

SRCS = ui_test.c

INCLUDEDIR = -I../input -I../fb -I../ui
LIBDIR = -L../input -L/home/zeus/src/nest/nestfs/lib -L../fb -L../ui
LIBS = -lnestinput -lfreetype -lfbdraw -luitoolkit

CFLAGS = -Wall -Werror -O2 $(INCLUDEDIR)
LDFLAGS = $(LIBDIR)
EXECNAME = ui_test

OBJS = $(SRCS:.c=.o)

ifdef CROSS_COMPILE
CC := $(CROSS_COMPILE)$(CC)
CXX := $(CROSS_COMPILE)$(CC)
endif

all:	$(OBJS)
	$(CC) $(LDFLAGS) -o $(EXECNAME) $(OBJS) $(LIBS)

clean:
	-rm -rf *.o *.so

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
