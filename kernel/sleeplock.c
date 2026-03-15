#include "sleeplock.h"
#include "schedule.h"
#include "wait_queue.h"
#include "libk/includes/stdio.h"

void sleeplock_init(sleeplock* slock) {
  slock->flag = 0; // lock not held
  wait_queue_init(&slock->queue);
}

void sleeplock_lock(sleeplock* slock) {
  if (!slock) {
    kprintf("Not a valid sleeplock.\n");
    return;
  }

  lock(&slock->queue.spinlock);
  while (slock->flag) { // sleep since lock is held
    wait_queue_sleep(&slock->queue, &slock->queue.spinlock);
    lock(&slock->queue.spinlock);
  }
  slock->flag = 1;
  unlock(&slock->queue.spinlock);
}

void sleeplock_unlock(sleeplock* slock) {
  if (!slock) {
    kprintf("Not a valid sleeplock.\n");
    return;
  }

  lock(&slock->queue.spinlock); 
  slock->flag = 0;
  wait_queue_wakeup(&slock->queue);
  unlock(&slock->queue.spinlock);
}
