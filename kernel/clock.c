#include "clock.h"
#include "../drivers/qemu/timer.h"

static uint64 boot_tick;

static inline uint64 read_cntpct() {
  uint64 val;
  asm volatile("mrs %0, CNTPCT_EL0" : "=r"(val));
  return val;
}

void clock_init() {
  boot_tick = read_cntpct();
}

uint64 get_clock_ticks() {
  return read_cntpct() - boot_tick;
}

int clock_get_time(clock_id_t id, timespec_t* ts) {
  switch (id) {
    case CLOCK_MONO : {
      uint64 elapsed = read_cntpct() - boot_tick;
      uint64 sec = (elapsed / clock_freq);
      uint64 remainder = (elapsed % clock_freq);
      uint64 nsec = (remainder * 1000000000) / clock_freq;
      ts->tv_sec = sec;
      ts->tv_nsec = nsec;
    }
      return 0;

    case CLOCK_REAL : {

    }
      return -1;

    default :

      return -1;
  }
}
