#include "types.h"

static inline uint64 read_elr_el1(void) {
    uint64 x;
    asm volatile("mrs %0, ELR_EL1" : "=r"(x));
    return x;
}

static inline uint64 read_spsr_el1(void) {
    uint64 x;
    asm volatile("mrs %0, SPSR_EL1" : "=r"(x));
    return x;
}

