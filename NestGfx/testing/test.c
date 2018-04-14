#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/fb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char * argv[])
{
  int fb = open("/dev/fd0", O_RDWR);

  if (fb < 0) {
    printf("Failed to open framebuffer!\n");
    exit(1);
  }

  if (argc != 6) {
    printf("usage: r g b wid hei\n");
    exit(1);
  }

  char r, g, b;
  int w, h;

  r = atoi(argv[1]);
  g = atoi(argv[2]);
  b = atoi(argv[3]);
  w = atoi(argv[4]);
  h = atoi(argv[5]);

  if (r < 0 || g < 0 || b < 0 || w < 0 || h < 0) {
    printf("Bad argument(s)\n");
    exit(0);
  }

  int i, j;

  for(i = 0; i < h; i++)
  {
    for(j = 0; j < w; j++)
    {
      char buf[4];
      buf[0] = b;
      buf[1] = g;
      buf[2] = r;
      buf[3] = 0;

      write(fb, buf, 4);
    }
  }

  close(fb);

  printf("done!\n");

  return 0;
}
