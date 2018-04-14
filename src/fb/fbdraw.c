#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/mman.h>
#include <string.h>

#include <linux/omapfb.h>

#include "fbdraw.h"

inline int getVarScreenInfo(void);
inline int getFixScreenInfo(void);
int panDisplay(uint32_t offset);

int fb_fildes;
uint32_t* fb_mem;					/* somehow this looks wrong */
uint8_t db;							/* double buffering available */
uint32_t db_offset;					/* secondary buffer offset */
struct fb_var_screeninfo fb_varinfo;
struct fb_fix_screeninfo fb_fixinfo;


int fb_init(const char* fbPath) {
	/* attempt to open framebuffer device */
	if((fb_fildes = open(fbPath, O_RDWR)) < 0) {
		perror("failed to attach framebuffer");
		return INT_MIN;
	}

	/* obtain framebuffer information */
	if((getVarScreenInfo()) < 0 ||
			(getFixScreenInfo() < 0)) {
#		ifndef NDEBUG
			perror("failed to obtain framebuffer info");
#		endif
		fb_end();
		return INT_MIN + 1;
	}

	/* map into framebuffer address space */
	if((fb_mem = (uint32_t*)
				mmap((void*)NULL,
					fb_fixinfo.smem_len,
					PROT_WRITE | PROT_READ,
					MAP_SHARED,
					fb_fildes,
					0)) == MAP_FAILED) {
#		ifndef NDEBUG
			perror("failed to map into framebuffer");
#		endif
		fb_end();
		return INT_MIN + 2;
	}

#	ifndef NDEBUG
		printf("got framebuffer at %#08lx with size %u KiB.\n",
				fb_fixinfo.mmio_start, fb_fixinfo.smem_len/1024);
#	endif

	/* check framebuffer type */
	if(fb_varinfo.yres == fb_varinfo.yres_virtual) {
		/* single buffering */
#		ifndef NDEBUG
			printf("framebuffer does not support double buffering.\n");
#		endif
		db = db_offset = 0;
	} else if((fb_varinfo.yres  << 1) >= fb_varinfo.yres_virtual) {
		/* double buffering available */
#		ifndef NDEBUG
			printf("framebuffer supports double buffering.\n");
#		endif
		db = 1;
		db_offset = fb_varinfo.yres;
		if(panDisplay(0) != 0) {
			/* can not pan, fall back to single buffering */
#			ifndef NDEBUG
				fprintf(stderr, "display can not pan, "
						"falling back to single buffering.\n");
#			endif
			db = db_offset = 0;
		}
	} else {
		/* unsupported type */
#		ifndef NDEBUG
			fprintf(stderr, "unhandled display geometry.\n");
#		endif
		fb_end();
		return INT_MIN + 3;
	}

#	ifndef NDEBUG
	printf(	"xres_virtual = %u\n"
			"yres_virtual = %u\n"
			"bits_per_pixel = %u\n"
			"xres = %u\n"
			"yres = %u\n"
			"line_length = %u\n"
			"xpanstep = %u\n"
			"ypanstep = %u\n"
			"left_margin = %u\n"
			"upper_margin = %u\n",
			fb_varinfo.xres_virtual,
			fb_varinfo.yres_virtual,
			fb_varinfo.bits_per_pixel,
			fb_varinfo.xres,
			fb_varinfo.yres,
			fb_fixinfo.line_length,
			fb_fixinfo.xpanstep,
			fb_fixinfo.ypanstep,
			fb_varinfo.left_margin,
			fb_varinfo.upper_margin);
#	endif

	/* application should have no direct access to
	 * framebuffer file descriptor */
	return 0;
}

void fb_end() {
	/* close file descriptor and unmap from memory */
	close(fb_fildes);
	munmap(fb_mem, fb_fixinfo.smem_len);
}

uint32_t fb_get_x_res() {
	return fb_varinfo.xres;
}

uint32_t fb_get_y_res() {
	return fb_varinfo.yres;
}

void fb_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
	*(fb_mem + y * (fb_varinfo.xres) + x) = color;
}

void fb_clear(void) {
	memset((void*)fb_mem, 0, (fb_varinfo.xres * fb_varinfo.yres) << 3);
}

/* internal stuff */
inline int getVarScreenInfo() {
	return ioctl(fb_fildes, FBIOGET_VSCREENINFO, &fb_varinfo);
}

inline int getFixScreenInfo() {
	return ioctl(fb_fildes, FBIOGET_FSCREENINFO, &fb_fixinfo);
}

int panDisplay(uint32_t offset) {
	int ioctlret;
	fb_varinfo.yoffset = offset;

	ioctlret = ioctl(fb_fildes, FBIOPAN_DISPLAY, &fb_varinfo);
	if(ioctlret < 0) {
#		ifndef NDEBUG
			perror("failed to pan display");
#		endif
		return ioctlret;
	}

	ioctlret = ioctl(fb_fildes, OMAPFB_WAITFORGO, 0);
#	ifndef NDEBUG
		if(ioctlret < 0) {
			perror("failed to wait for vsync");
		}
#	endif

	return ioctlret;
}
