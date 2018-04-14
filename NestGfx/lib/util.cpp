#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Logging Methods
*/

void log_error(const char * fmt, ...)
{
  va_list va;

  va_start(va, fmt);

  fprintf(stderr, "ERROR: ");
  vfprintf(stderr, fmt, va);
  fprintf(stderr, "\n");

  va_end(va);
}

void log_errno(const char * fmt, ...)
{
  va_list va;

  int errnoSaved = errno;

  va_start(va, fmt);

  fprintf(stderr, "ERROR: ");
  vfprintf(stderr, fmt, va);

  if(errnoSaved)
    fprintf(stderr, " (%s)\n", strerror(errnoSaved));

  va_end(va);
}

void log_info(const char * fmt, ...)
{
  va_list va;

  va_start(va, fmt);

  fprintf(stderr, "INFO: ");
  vfprintf(stderr, fmt, va);
  fprintf(stderr, "\n");

  va_end(va);
}

void log_debug(const char * fmt, ...)
{
  va_list va;

  va_start(va, fmt);

  fprintf(stderr, "DEBUG: ");
  vfprintf(stderr, fmt, va);
  fprintf(stderr, "\n");

  va_end(va);
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

/* Unix Misc
*/
void background()
{  
  // fork to background
  if(daemon(1, 0) < -1)
  {
    log_error("Failed to move to background");
    die();
  }

  log_info("Forked to background as %d", getpid());
}

bool write_pid_file(const char * filename)
{  
  // write pid file
  FILE * fp;
  
  fp = fopen(filename, "r");

  // already exists, kill the running process
  if(fp)
  {
    pid_t pid = 0;

    fscanf(fp, "%d", &pid);

    if(pid != 0)
    {
      // signal sent
      if(kill(pid, SIGTERM) == 0)
      {
        log_info("Ending existing process %d", pid);
        usleep((int)5e5); // yeah...500ms
      }
    }
  }

  // now get ready to write
  fp = fopen(filename, "w");

  if(!fp)
    return false;

  if(fprintf(fp, "%d", getpid()) < 0)
    return false;

  fclose(fp);

  return true;
}

void die()
{
  exit(EXIT_FAILURE);
}
