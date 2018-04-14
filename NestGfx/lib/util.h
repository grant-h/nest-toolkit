#ifndef UTIL_H
#define UTIL_H

void log_error(const char * fmt, ...);
void log_errno(const char * fmt, ...);
void log_info(const char * fmt, ...);
void log_debug(const char * fmt, ...);

int randInt(int start, int end);
float randFloat(float start, float end);

void background();
bool write_pid_file(const char * filename);

void die();

#define BUG() \
  log_error("Bug in %s:%s() at line %d", __FILE__, __func__, __LINE__); \
  die(); 

#endif
