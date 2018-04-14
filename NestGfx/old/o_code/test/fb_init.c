#include <stdlib.h>
#include <assert.h>

#include "../fb.h"

int main(int argc, char** argv) {
	if(argc == 1) return EXIT_FAILURE;

	assert(fb_init(argv[1]) == 0);
	fb_end();

	return EXIT_SUCCESS;
}
