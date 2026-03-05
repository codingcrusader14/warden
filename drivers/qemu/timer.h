/*
 * timer.h - Armv8-A Generic Timer (QEMU virt)
 * 
 * Provides timer initlization, tick configuration, and tick rearmament
 * for preemptive scheduling. 
 */

#ifndef QEMU_TIMER_H
#define QEMU_TIMER_H

#include "../../kernel/types.h"

#define TIMER_INTERVAL_MS 10
#define ENABLE_TIMER (1 << 0)
#define MASK_INTTERUPT (1 << 1)
#define UNMASK_INTERRUPT 0x2

extern uint64 clock_freq;

void disable_interrupts();
void enable_interrupts();
void timer_init();
void timer_rearm();

#endif
