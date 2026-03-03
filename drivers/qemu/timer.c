#include "timer.h"
#include "../../kernel/global.h"

static uint64 ticks_per_interval;

static inline uint64 read_cntfrq() {
  uint64 val; 
  asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(val));
  return val;
}

static inline void write_cntp_tval(uint64 val) {
  asm volatile("msr S3_3_C14_C2_0, %0"
                :
                : "r" (val)
              );
}

static inline void write_cntp_ctl(uint64 val) {
   asm volatile("msr S3_3_C14_C2_1, %0"
                :
                : "r" (val)
              );
}

static inline void write_daif_clr() {
   asm volatile("msr DAIFCLr, #2");
}

void disable_interrupts() {
   asm volatile("msr DAIFSet, #0xF");
}

void enable_interrupts() {
   asm volatile("msr DAIFCLr, #0xF");
}

void timer_init() {
  uint64 freq = read_cntfrq(); // read frequency
  ticks_per_interval = (freq / 100); // 10ms
  write_cntp_tval(ticks_per_interval); // arm countdown
  write_cntp_ctl(ENABLE_TIMER); // enable timer

}

void timer_rearm() {
  write_cntp_tval(ticks_per_interval); // arm countdown
  global_tick++;
}

