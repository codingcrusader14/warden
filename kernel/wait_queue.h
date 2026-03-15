#ifndef WAIT_QUEUE_H
#define WAIT_QUEUE_H

#include "process.h"
#include "spinlock.h"

typedef struct {
  task_t* head;
  task_t* tail;
  lock_t spinlock;
} wait_queue;

void wait_queue_init(wait_queue* queue);
void wait_queue_sleep(wait_queue* queue, lock_t* spinlock);
task_t* wait_queue_wakeup(wait_queue* queue);


#endif
