#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define clear_obj(o) memset((o), 0, sizeof(*(o)))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#endif
