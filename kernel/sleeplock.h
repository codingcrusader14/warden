#ifndef SLEEPLOCK_H
#define SLEEPLOCK_H

#include "wait_queue.h"
#include "spinlock.h"
#include "types.h"

typedef struct {
  uint32 flag;
  wait_queue queue;
} sleeplock;

void sleeplock_init(sleeplock* slock);
void sleeplock_lock(sleeplock* slock);
void sleeplock_unlock(sleeplock* slock);

#endif
