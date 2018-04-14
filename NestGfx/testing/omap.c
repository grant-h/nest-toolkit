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
#include <signal.h>

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

struct fb_var_screeninfo var;
struct fb_fix_screeninfo fix;
void * gFbmem = NULL;

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

//static int fb_update_window(int fd, short x, short y, short w, short h)
//{
//	struct omapfb_update_window uw;

//	uw.x = x;
//	uw.y = y;
//	uw.width = w;
//	uw.height = h;

	//printf("update %d,%d,%d,%d\n", x, y, w, h);
//	FBCTL(OMAPFB_UPDATE_WINDOW, &uw);
//	FBCTL0(OMAPFB_SYNC_GFX);

//	return 0;
//}

static int draw_pixel(void *fbmem, int x, int y, unsigned color)
{
	unsigned h = var.yres_virtual;
	unsigned w = var.xres_virtual;

  if(x > w-1)
  {
    //printf("%s: x coord out of range (%d)\n", __FUNCTION__, x);
    return 1;
  }

  if(y > h-1)
  {
    //printf("%s: y coord out of range (%d)\n", __FUNCTION__, y);
    return 1;
  }

	if (var.bits_per_pixel == 16) {
		unsigned short c;
		unsigned r = (color >> 16) & 0xff;
		unsigned g = (color >> 8) & 0xff;
		unsigned b = (color >> 0) & 0xff;
		unsigned short *p;

		r = r * 32 / 256;
		g = g * 64 / 256;
		b = b * 32 / 256;

		c = (r << 11) | (g << 5) | (b << 0);

		fbmem += fix.line_length * y;

		p = fbmem;

		p += x;

		*p = c;
	} else {
		unsigned int *p;

		fbmem += fix.line_length * y;

		p = fbmem;

		p += x;

                //printf("  fbmem %x: ptr %x color %x\n", fbmem, p, color);
		*p = color;
	}

        return 0;
}

void clear_screen(void * fbmem, unsigned color)
{
  unsigned x, y;
  unsigned h = var.yres_virtual;
  unsigned w = var.xres_virtual;

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      draw_pixel(fbmem, x, y, color);
    }
  }
}

void fill_screen(void *fbmem)
{
	unsigned x, y;
	unsigned h = var.yres_virtual;
	unsigned w = var.xres_virtual;
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

struct circleState {
  unsigned x;
  unsigned y;
  int xdir;
  int ydir;
  unsigned hL; 
  unsigned hR; 
  unsigned hT;
  unsigned hB; 
  unsigned sleeper;
  unsigned time;
  bool diag;
  unsigned color;
};

void circle_state_reset(struct circleState * s, unsigned color,
    int speed, bool orient)
{
  memset(s, 0, sizeof(*s));

	unsigned h = var.yres_virtual;
	unsigned w = var.xres_virtual;

  s->diag = orient;
  s->color = color;
  s->time = speed;

  // diagonal
  if(orient)
  {
    s->x = w/2;
    s->time /= 2;
  }
  else
  {
    s->x = 0;
  }

  s->y = 0;
  s->xdir = 1;
  s->hT = 1;

  if(orient)
    s->ydir = 1;
}

bool circle_tick(void *fbmem, struct circleState * state)
{
	unsigned h = var.yres+320;
	unsigned w = var.xres_virtual;
	unsigned x = state->x, y = state->y;
  bool orient = state->diag;

  int xdir = state->xdir, ydir = state->ydir;
  unsigned hL = state->hL, 
           hR = state->hR, 
           hT = state->hT,
           hB = state->hB; 

  int sleeper = state->sleeper;

  if(sleeper <= state->time)
  {
    state->sleeper++;
    return false;
  }

  //printf("draw %u %u\n", x,y);

  if(xdir == 1 && x+1 >= w-hR) {
    if(orient)
      xdir = -1;
    else
      xdir = 0;
    ydir = 1;
    hR++;
  } else if(ydir == 1 && y+1 >= h-hB) { // bottom
    xdir = -1;
    if(orient)
      ydir = -1;
    else
      ydir = 0;
    hB++;
  } else if(xdir == -1 && (int)x-1 <= hL) { // left
    if(orient)
      xdir = 1;
    else
      xdir = 0;
    ydir = -1;
    hL++;
  } else if(ydir == -1 && (int)y-1 <= hT) {
    xdir = 1;
    if(orient)
      ydir = 1;
    else
      ydir = 0;
    hT++;
  }

  draw_pixel(fbmem, x, y, state->color);

  if(x != 0 || xdir > -1)
    x += xdir;
  if(y != 0 || ydir > -1)
    y += ydir;

  state->x = x;
  state->y = y;
  state->xdir = xdir;
  state->ydir = ydir;
  state->hL = hL; 
  state->hR = hR; 
  state->hT = hT;
  state->hB = hB; 
  state->sleeper = sleeper;

  return !(hL*2 < w && hB*2 < h);
}

void circle_in(void *fbmem, unsigned color, int time, int orient)
{
	unsigned h = var.yres_virtual;
	unsigned w = var.xres_virtual;
	unsigned x = 0, y = 0;

  // diagonal
  if(orient)
  {
    x = w/2;
    time /= 2;
  }
  
  int xdir = 1, ydir = 0;
  unsigned hL = 0, hR = 0, hT = 1, hB = 0; 

  if(orient)
    ydir = 1;

  // calculate the time needs for the animation
  float timeStep = (float)time/(w*h);
  useconds_t uSec = timeStep*1e6;
  int sleeper = 0;

  while (hL*2 < w && hB*2 < h) {
    // hit right wall 
    if(xdir == 1 && x+1 >= w-hR) {
      if(orient)
        xdir = -1;
      else
        xdir = 0;
      ydir = 1;
      hR++;
    } else if(ydir == 1 && y+1 >= w-hB) { // bottom
      xdir = -1;
      if(orient)
        ydir = -1;
      else
        ydir = 0;
      hB++;
    } else if(xdir == -1 && x-1 <= hL) { // left
      if(orient)
        xdir = 1;
      else
        xdir = 0;
      ydir = -1;
      hL++;
    } else if(ydir == -1 && y-1 <= hT) {
      xdir = 1;
      if(orient)
        ydir = 1;
      else
        ydir = 0;
      hT++;
    }

    draw_pixel(fbmem, x, y, color);

    if(sleeper > time)
    {
      struct timespec req;
      req.tv_sec = 0;
      req.tv_nsec = 0;

      clock_nanosleep(CLOCK_REALTIME, 0, &req, NULL);
      sleeper = 0;
    } else sleeper++;

    if(x != 0 || xdir > -1)
      x += xdir;
    if(y != 0 || ydir > -1)
      y += ydir;
  }
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

void draw_circle(void * fbmem, unsigned x, unsigned y, unsigned rad, unsigned color)
{
	unsigned h = var.yres_virtual;
	unsigned w = var.xres_virtual;
  int i, j;

  printf("circle (%u %u) %u\n", x, y, rad);

  for(i = max((int)y-rad, 0); i < min(y+rad+1, h); i++)
  {
    int y2 = ((int)i-y)*((int)i-y);

    for(j = max((int)x-rad, 0); j < min(x+rad+1, w); j++) 
    {
      int x2 = (int)j-x;
      float dist = sqrt(x2*x2+y2);


      if(dist <= rad) {
        float darkener = rad - dist;
        
        if(darkener < 0.0)
          darkener = 0.0;

        darkener = darkener/rad;

        unsigned newColor = dim(color, darkener);
        draw_pixel(fbmem, j, i, newColor);
      }
    }
  }

}


// [start, end)
int randInt(int start, int end)
{
  if(end < start)
    return 0;

  float r = (float)rand()/RAND_MAX;

  return (start + (end-start)*r);
}

// [start, end)
float randFloat(float start, float end)
{
  if(end < start)
    return 0.0;

  float r = (float)rand()/RAND_MAX;

  return (start + (end-start)*r);
}

struct tumbleState
{
  float radius;
  float radiusStep;
  float initrad;
  float angle;
  unsigned color;
  unsigned nextColor;
  float colorStep;
  unsigned delay;
  unsigned counter;
};

unsigned lerp(unsigned c1, unsigned c2, float slider)
{
  unsigned char r1 = c1 & 0xff;
  unsigned char g1 = c1>>8 & 0xff;
  unsigned char b1 = c1>>16 & 0xff;
  unsigned char r2 = c2 & 0xff;
  unsigned char g2 = c2>>8 & 0xff;
  unsigned char b2 = c2>>16 & 0xff;

  int dr = (int)r1 - (int)r2;
  int dg = (int)g1 - (int)g2;
  int db = (int)b1 - (int)b2;

  int slide = slider*10000;

  printf("%d ", slide);

  unsigned char r = (char)r1 + (int)(dr*slide)/10000;
  unsigned char g = (char)g1 + (int)(dg*slide)/10000;
  unsigned char b = (char)b1 + (int)(db*slide)/10000;

  printf("%hhu\n", r);

  return (r << 16) | (b << 8) | g;
}

bool tumble_tick(void * fbmem, struct tumbleState * s)
{
  unsigned h = var.yres;
  unsigned w = var.xres;
  int i, j;

  unsigned cX = w/2;
  unsigned cY = h/2;

#define PI 3.1415926535f
#define DEG2RAD(x) ((float)(x)*PI/180)

  if(s->counter++ < s->delay)
    return false;
  else
    s->counter = 0;

  // get to outer edge
  float radius = s->radius; //cX-1;
  float angle = s->angle; //180.0; // in degrees
  float initrad = s->initrad;

  // step the angle
  // increase the speed as we get lower down
  float step = 0.01 + 0.02*((initrad - radius)/(initrad));

  angle = fmodf(angle + step, 360.0);

  if(angle < 180.2 && angle > 179.8) // we wrapped
  {
    radius -= s->radiusStep;
    //angle = 180.21;
  }

  int x = radius*cosf(DEG2RAD(angle)) + cX;
  int y = radius*sinf(DEG2RAD(angle)) + cX;

  // TODO: look in to how it's being double buffered
  if(draw_pixel(fbmem, x, y, s->color))
    return true; //yup we're done

  s->radius = radius;
  s->angle = angle;

  return !(radius > 0.01);
}

void exitFunc(int sig)
{
  clear_screen(gFbmem, 0);
  exit(0);
}

static void fpsCount()
{
  static struct timeval start; 
  static int ticks; 

  if(ticks == 0) {
    gettimeofday(&start, NULL);
    ticks++;
  } else {
    struct timeval delta;
    struct timeval now;

    ticks++;
    gettimeofday(&now, NULL);
    timersub(&now, &start, &delta);

    if(delta.tv_sec >= 1) {
      fprintf(stderr, "Ticks/s %d\n", ticks);
      ticks = 0;
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

  if(argc == 2 && !strcmp(argv[1], "debug"))
  {
    printf("DEBUG\n");
  }
  else
  {
    // fork to background
    if(daemon(1, 0) < -1)
    {
      printf("Failed to move to background\n");
      exit(1);
    }

    printf("Forked to background as %d\n", getpid());
  }


  // write pid file
  FILE * fp = fopen("/tmp/killme.pid", "w");
  fprintf(fp, "%d", getpid());
  fclose(fp);

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
  printf("accel %d\n", fix.accel);
  fprintf(stderr, "X Panning: %s\n", fix.xpanstep ? "hw" : "sw");
  fprintf(stderr, "Y Panning: %s\n", fix.ypanstep ? "hw" : "sw");
  fprintf(stderr, "Y Wrap: %s\n", fix.ywrapstep ? "hw" : "sw");

  void* ptr = mmap(0, var.yres_virtual * fix.line_length,
      PROT_WRITE | PROT_READ,
      MAP_SHARED, fd, 0);

  if(ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  gFbmem = ptr;

  signal(SIGINT, exitFunc);

  /*fill_screen(ptr);
    sleep(1);*/
  clear_screen(ptr, 0);
  printf("Starting draw\n");

  int i;
  int speed = 200;
  unsigned color = 0xff;
  int orient = 0;

  int amount = 4;
  int delay = 0;
  struct circleState * cstates;
  struct tumbleState * tstates;
  bool * isDone;

  cstates = calloc(amount, sizeof(struct circleState));
  tstates = calloc(amount, sizeof(struct tumbleState));
  isDone  = calloc(amount, sizeof(bool));

  if(!cstates || !tstates)
  {
    printf("memory!\n");
    exit(1);
  }

  for(i = 0; i < amount; i++)
  {
    color = randColor();
    circle_state_reset(&cstates[i], color, randInt(0, (int)75e3), orient);

    tstates[i].radius = 160-1-10*i;
    tstates[i].initrad = tstates[i].radius;
    tstates[i].radiusStep = randFloat(0.5, 0.9);
    tstates[i].angle = 180.0;
    tstates[i].color = color;
    tstates[i].counter = 0;
  }

  while(1) {
    /* for(i = 0; i < amount; i++) {
       if(circle_tick(ptr, &states[i])) {
       color = randColor();
       printf("new circle %x\n", color);
       clear_screen(ptr, randColor());
       circle_state_reset(&states[i], color, randInt(0, (int)75e3), orient);
       }
       }*/

    for(i = 0; i < amount; i++) {
      if(!isDone[i] && tumble_tick(ptr, &tstates[i])) {
        /*color = randColor();
        printf("new tumble %x\n", color);*/

        /*tstates[i].radius = 160-1-10*i;
        tstates[i].angle = 180.0;
        tstates[i].color = color;
        tstates[i].counter = 0;*/
        //clear_screen(ptr, 0x0);
        isDone[i] = true;
      }
    }

    bool allDone = true;

    for(i = 0; i < amount; i++)
      if(!isDone[i])
        allDone = false;

    if(allDone)
    {
      clear_screen(ptr, 0x0);
      for(i = 0; i < amount; i++) {
        color = randColor();
        printf("new tumble %x\n", color);

        tstates[i].radius = 160-1-10*i;
        tstates[i].initrad = tstates[i].radius;
        tstates[i].radiusStep = randFloat(0.5, 0.9);
        tstates[i].angle = 180.0;
        tstates[i].color = color;
        tstates[i].counter = 0;
        isDone[i] = false;
      }
    }

    fpsCount();

    if(delay > 10000)
    {
      struct timespec req;
      req.tv_sec = 0;
      req.tv_nsec = 0;

      //clock_nanosleep(CLOCK_REALTIME, 0, &req, NULL);
      delay = 0;

      //draw_circle(ptr, 160, 480, 100.0, 0xff0000);
      //tmpAmount = min(tmpAmount + 1, amount);
    } else delay++;
  }

  /*while(1) {
    draw_circle(ptr, randInt(0,w), randInt(0,h), randInt(10,30), randColor());
    usleep(10000);
    }*/

  //	FBCTL(OMAPFB_GET_UPDATE_MODE, &update_mode);
  //	if (update_mode == OMAPFB_MANUAL_UPDATE)
  //		fb_update_window(fd, 0, 0, di.xres, di.yres);

  return 0;

}
