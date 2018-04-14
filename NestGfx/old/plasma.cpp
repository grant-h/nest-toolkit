#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/fb.h>

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define ERROR(x) printf("fbtest error in line %s:%d: %s\n", __FUNCTION__, __LINE__, strerror(errno));

#define FBCTL(cmd, arg)			\
	if(ioctl(fd, cmd, arg) == -1) {	\
		ERROR("ioctl failed");	\
		exit(1); }

#define FBCTL0(cmd)			\
	if(ioctl(fd, cmd) == -1) {	\
		ERROR("ioctl failed");	\
		exit(1); }

struct vec2 {
  float x, y;
};

struct vec3 {
  float x, y, z;
};

struct vec4 {
  float x, y, z, w;
};


struct fb_var_screeninfo var;
struct fb_fix_screeninfo fix;

int open_fb(const char* dev)
{
	int fd = -1;
	fd = open(dev, O_RDWR);
	if(fd == -1)
	{
		printf("Error opening device %s : %s\n", dev, strerror(errno));
		exit(-1);
	}

	return fd;
}

unsigned int randColor()
{
  return rand();
}

unsigned dim(unsigned color, float dim)
{
  unsigned char r = color & 0xff;
  unsigned char g = color>>8 & 0xff;
  unsigned char b = color>>16 & 0xff;

  r *= dim;
  g *= dim;
  b *= dim;

  return (b << 16) | (g << 8) | r;
}

// [start, end)
int randInt(int start, int end)
{
  if(end < start)
    return 0;

  float r = (float)rand()/RAND_MAX;

  return (start + (end-start)*r);
}

static void draw_line(void *fbmem, int y, vec3 * color)
{
  unsigned int *p;
  unsigned int * mem = (unsigned int *)malloc(fix.line_length);

  unsigned int offset = fix.line_length * (y+(var.yres-var.yoffset));

  fbmem += offset;

  p = reinterpret_cast<unsigned int *>(fbmem);

  int i;
  for(i = 0; i < var.xres; i++)
  {
    mem[i] = ((int)(color[i].z*255) << 16)
            | ((int)(color[i].y*255) << 8)
            | ((int)(color[i].x*255));
  }

  memcpy(p, mem, fix.line_length);

  free(mem);
}


static void draw_pixel(void *fbmem, int x, int y, vec3 color)
{
  if(x >= var.xres)
  {
    printf("x out of range\n");
    exit(1);
  }

  if(y >= var.yres)
  {
    printf("y out of range\n");
    exit(1);
  }

  unsigned int *p;
  unsigned int offset = fix.line_length * (y+(var.yres-var.yoffset));

  if(offset >= fix.smem_len)
  {
    printf("dragons\n");
    exit(1);
  }

  p = reinterpret_cast<unsigned int *>(fbmem+offset);
  p += x;

  *p = ((int)(color.x*255) << 16) |  // R
       ((int)(color.y*255) <<  8) |  // G
       ((int)(color.z*255));         // B
}

static void draw_pixel(void *fbmem, int x, int y, unsigned color)
{
  vec3 v = {(color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff};
  draw_pixel(fbmem, x, y, v);
}

void fill_screen(void *fbmem)
{
  unsigned x, y;
  unsigned h = var.yres;
  unsigned w = var.xres;
  //	int color;

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      if (x < 20 && y < 20)
        draw_pixel(fbmem, x, y, 0xffffff);
      else if (x < 20 && (y > 20 && y < h - 20))
        draw_pixel(fbmem, x, y, 0xff);
      else if (y < 20 && (x > 20 && x < w - 20))
        draw_pixel(fbmem, x, y, 0xff00);
      else if (x > w - 20 && (y > 20 && y < h - 20))
        draw_pixel(fbmem, x, y, 0xff0000);
      else if (y > h - 20 && (x > 20 && x < w - 20))
        draw_pixel(fbmem, x, y, 0xffff00);
      else if (x == 20 || x == w - 20 ||
          y == 20 || y == h - 20)
        draw_pixel(fbmem, x, y, 0xffffff);
      else if (x == y || w - x == h - y)
        draw_pixel(fbmem, x, y, 0xff00ff);
      else if (w - x == y || x == h - y)
        draw_pixel(fbmem, x, y, 0x00ffff);
      else if (x > 20 && y > 20 && x < w - 20 && y < h - 20) {
        int t = x * 3 / w;
        unsigned r = 0, g = 0, b = 0;
        unsigned c;
        if (var.bits_per_pixel == 16) {
          if (t == 0)
            b = (y % 32) * 256 / 32;
          else if (t == 1)
            g = (y % 64) * 256 / 64;
          else if (t == 2)
            r = (y % 32) * 256 / 32;
        } else {
          if (t == 0)
            b = (y % 256);
          else if (t == 1)
            g = (y % 256);
          else if (t == 2)
            r = (y % 256);
        }
        c = (r << 16) | (g << 8) | (b << 0);
        draw_pixel(fbmem, x, y, c);
      } else {
        draw_pixel(fbmem, x, y, 0);
      }
    }

  }
}


vec3 palette(float pos, float time)
{
  struct vec3 v3 = {sinf(3.14/2.0*pos),cosf(6.2*pos), cosf(pos+time/10.0)};

  return v3; 
}

void do_plasma(void * fbmem, float t)
{
  unsigned x, y;
  unsigned h = var.yres;
  unsigned w = var.xres;

  vec3 * data = (vec3 *)malloc(sizeof(vec3)*w);
  int skip = 0;

  for(y = 0; y < h; y++)
  {
    float plas;
    skip = 0;
    for(x = 0; x < w; x++)
    {
      if(!skip)
      {
        vec2 p = {(float)x/w, (float)y/h};

        // translate
        p.y += 0.1;
        // zoom to a good looking spot
        p.x *= 0.25;
        p.y *= 0.25;

        // start the plasma magic!
        float part1 = sinf(p.x*(90+21*cosf(p.y))+t);
        float part2 = cosf(p.y*(32+11*sinf(p.x*57))+t);
        float part3 = sinf(p.x*(55 + 21*sinf(p.y*32))+ t);
        plas = 0.5 + 0.65*part1*part2 + 0.35*part3*part2;

        data[x] = palette(plas, t);
      }

      if(skip)
        data[x] = data[x-1];

      if(skip < 1)
        skip++;
      else
        skip = 0;
    }

    draw_line(fbmem, y, data);
  }

  free(data);
}

void flip_buffer(int fd)
{
  if(var.yoffset == 0) {
    var.yoffset = 320;
    FBCTL(FBIOPAN_DISPLAY, &var);
  } else if(var.yoffset == 320) {
    var.yoffset = 0;
    FBCTL(FBIOPAN_DISPLAY, &var);
  } else {
    printf("Bad yoffset\n");
    exit(1);
  }
}

void clear_screen(void * fbmem, unsigned color)
{
  unsigned x, y;
  unsigned h = var.yres;
  unsigned w = var.xres;

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      draw_pixel(fbmem, x, y, color);
    }
  }
}

int main(int argc, char** argv)
{
  int fb_num;
  char str[64];
  int fd;
  //	enum omapfb_update_mode update_mode;
  //	struct omapfb_display_info di;

  srand(time(NULL));
  
  // write pid file
  FILE * fp = fopen("/tmp/killme.pid", "w");
  fprintf(fp, "%d", getpid());
  fclose(fp);

  if (argc == 2)
    fb_num = atoi(argv[1]);
  else
    fb_num = 0;

  sprintf(str, "/dev/fb%d", fb_num);
  printf("opening %s\n", str);

  fd = open(str, O_RDWR);

  //	FBCTL(OMAPFB_GET_DISPLAY_INFO, &di);

  FBCTL(FBIOGET_VSCREENINFO, &var);
  FBCTL(FBIOGET_FSCREENINFO, &fix);

  printf("res %d,%d virtual %d,%d, line_len %d\n",
      var.xres, var.yres,
      var.xres_virtual, var.yres_virtual,
      fix.line_length);
  printf("dim %d,%d\n", var.width, var.height);
  printf("bpp %d\n", var.bits_per_pixel);

  void* ptr = mmap(0, var.yres_virtual * fix.line_length,
      PROT_WRITE | PROT_READ,
      MAP_SHARED, fd, 0);

  if(ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  float t = 0.0;

  fill_screen(ptr);
  flip_buffer(fd);

  while(1)
  {
    do_plasma(ptr, t);
    flip_buffer(fd);

    printf("%f\n", t);

    t += 0.1;
  }

  return 0;
}
