#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#include "util.h"
#include "Gfx.h"
#include "Timer.h"

Gfx * g_fb = NULL;

void exitFunc(int sig)
{
  if(g_fb)
  {
    delete g_fb;
    g_fb = NULL;
  }

  exit(0);
}

int main(int argc, char * argv[])
{
  const char * fbPath = getenv("FRAMEBUFFER");
  const char * pidFile = "/tmp/killme.pid";

  signal(SIGINT, exitFunc);
  signal(SIGTERM, exitFunc);

  if(!fbPath)
    fbPath = "/dev/fb0";

  if(argc >= 2 && strcmp(argv[1], "debug") == 0)
    log_info("Starting in interactive mode");
  else
    background();

  if(!write_pid_file(pidFile))
  {
    log_error("Failed to write our PID file %s", pidFile);
    die();
  }

  Gfx * fb = new Gfx(fbPath);

  srand(time(NULL));

  if(!fb->attach())
    die();

  // for cleanup
  g_fb = fb;

  // clean off the crud
  fb->drawClear(COLOR_BLACK);
  fb->flip();

  delete fb;

  return 0;
}
