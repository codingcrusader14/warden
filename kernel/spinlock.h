#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

typedef struct {
  uint32 flag;
} lock_t;

void lock_init(lock_t* mutex);
void lock(lock_t* mutex);
void unlock(lock_t* mutex);
void acquire(lock_t* mutex);
void release(lock_t* mutex);
void lock_irqsave(lock_t* mutex, uint64* flags);
void unlock_irqrestore(lock_t* mutex, uint64 flags);

#endif
