#ifndef SCHEDULE_H
#define SCHEDULE_H

extern struct task* current_task;

void schedule();
void scheduler_add(struct task *t);

#endif
