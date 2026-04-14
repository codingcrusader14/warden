#include "spinlock.h"
#include "process.h"
#include "../drivers/qemu/timer.h"

/* Checks which core we are running on, bottom bits give affinity level*/
uint64 cpu_id() {
  uint64 mpidr;
  asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
  return mpidr & 0xff;
}

/* Checks current state of our interrupt image */
uint64 read_interrupt_mask() {
  uint64 daif;
  asm volatile("mrs %0, daif" : "=r"(daif));
  return daif;
}

/* Writes back the interrupt image */
void write_interrupt_mask(cpu* c) {
  uint64 daif = c->exception_state;
  asm volatile("msr daif, %0" :: "r"(daif));
}

void lock_init(lock_t* mutex) {
  // 0 unlocked, 1 locked;
  mutex->flag = 0;
}

void lock(lock_t* mutex) {
  cpu* c = &cpus[cpu_id()]; 

  // we check if this is the first lock acquire in order to avoid deadlock
  // if we dont disable interrupts we may get preempted and try to accquire the lock again
  if (c->nested_locks == 0) {
    c->exception_state = read_interrupt_mask();
    disable_interrupts();

  }
  c->nested_locks++;
  acquire(mutex);
}


void unlock(lock_t* mutex) {
  cpu* c = &cpus[cpu_id()];
  release(mutex);
  c->nested_locks--;

  // we check if this is the last lock in where we can restore the interrupt state
  if (c->nested_locks == 0) {
    write_interrupt_mask(c);
  }
}

static inline uint64 save_daif(void) {
    uint64 flags;
    asm volatile("mrs %0, daif" : "=r"(flags));
    return flags;
}

static inline void restore_daif(uint64 flags) {
    asm volatile("msr daif, %0" :: "r"(flags));
}

void lock_irqsave(lock_t* mutex, uint64* flags) {
    *flags = save_daif();
    disable_interrupts();
    acquire((lock_t*)&mutex->flag);
}

void unlock_irqrestore(lock_t *mutex, uint64 flags) {
    release((lock_t*)&mutex->flag);
    restore_daif(flags);
}
