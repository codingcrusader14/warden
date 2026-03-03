#ifndef SCHEDULE_H
#define SCHEDULE_H

extern struct task* current_task;

void kexit();
void yield();
void scheduler_add(struct task *t);
int schedule();
int scheduler_start();

#endif
