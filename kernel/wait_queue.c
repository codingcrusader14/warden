#include "wait_queue.h"
#include "schedule.h"
#include "spinlock.h"

static void wait_queue_enqueue(wait_queue* queue, task_t* t) {
  t->next_wait = NULL;

  if (queue->tail) 
    queue->tail->next_wait = t;
  else 
    queue->head = t;

  queue->tail = t;
}

static task_t* wait_queue_dequeue(wait_queue* queue) {
  task_t* t = queue->head;

  if (!t)
    return NULL;

  queue->head = queue->head->next_wait;
  if (!queue->head) 
    queue->tail = NULL;

  t->next_wait = NULL;
  return t;
}

void wait_queue_init(wait_queue* queue) {
  queue->head = NULL;
  queue->tail = NULL;
}

void wait_queue_sleep(wait_queue* queue, lock_t* spinlock) {
  if (!queue || !current_task) return; 

  wait_queue_enqueue(queue, current_task);
  sleep(spinlock); // caller responsible for locking unlocking
}

task_t* wait_queue_wakeup(wait_queue* queue) {
  if (!queue) return NULL;

  task_t* t = wait_queue_dequeue(queue);

  if (t) 
    wakeup(t);
  return t;
}
