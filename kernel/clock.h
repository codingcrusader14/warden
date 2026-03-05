#ifndef CLOCK_H
#define CLOCK_H

#include "types.h" 

#define CLOCK_MONO 0
#define CLOCK_REAL 1

typedef int clock_id_t;

typedef struct {
  uint64 tv_sec;
  uint64 tv_nsec;
} timespec_t;

void clock_init();
uint64 get_clock_ticks();
int clock_get_time(clock_id_t, timespec_t* ts);

#endif
