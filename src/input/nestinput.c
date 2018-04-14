#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>

#include "nestinput.h"

#define OPTNAV "/dev/input/event1"
#define BUTTON "/dev/input/event2"

int ni_read(int fildes);

typedef struct readval_ {
	uint32_t seconds;
	uint32_t useconds;
	uint32_t unknown;
	int32_t diff;
	char other[16];
} readval;

int rotary, button;

int ni_init(void) {
	int flags;
	/* attempt to open event device for optical navigation module */
	printf("sizeof(readval)=%d\n", sizeof(readval));
	rotary = open(OPTNAV, O_RDONLY);
	if(rotary < 0) {
		perror("open " OPTNAV);
		return rotary;
	}
	/* and attempt to set non-blocking reads */
	flags = fcntl(rotary, F_GETFL, 0);
	if(flags < 0) {
		perror("fcntl F_GETFL " OPTNAV);
		return flags;
	}
	flags = fcntl(rotary, F_SETFL, flags | O_NONBLOCK);
	if(flags < 0) {
		perror("fcntl F_SETFL " OPTNAV);
		return flags;
	}
	
	/* attempt to open event device for button */
	button = open(BUTTON, O_RDONLY);
	if(button < 0) {
		perror("open " BUTTON);
		return button;
	}
	/* and attempt to set non-blocking reads */
	flags = fcntl(button, F_GETFL, 0);
	if(flags < 0) {
		perror("fcntl F_GETFL " BUTTON);
		return flags;
	}
	flags = fcntl(button, F_SETFL, flags | O_NONBLOCK);
	if(flags < 0) {
		perror("fcntl F_SETFL " BUTTON);
		return flags;
	}

	return 0;
}

int ni_read_scroll(void) {
	return ni_read(rotary) / 32;
}

int ni_read_button(void) {
	return ni_read(button);
}

int ni_read(int fildes) {
	readval r;
	int t;

	t = read(fildes, (void*)&r, 32);
	if(t != 32 || t < 0) {
		if(errno != EAGAIN)
			perror("read button");
		return 0;
	}

	return r.diff;
}
