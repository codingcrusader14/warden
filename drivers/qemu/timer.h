#ifndef QEMU_TIMER_H
#define QEMU_TIMER_H

#define TIMER_INTERVAL_MS 10
#define ENABLE_TIMER (1 << 0)
#define MASK_INTTERUPT (1 << 1)
#define UNMASK_INTERRUPT 0x2

void disable_interrupts();
void enable_interrupts();
void timer_init();
void timer_rearm();

#endif
