#ifndef DEFS_H
#define DEFS_H

#include "types.h"
#include <stdbool.h>

#define STACK_SIZE 4080
#define NCPU 4 // Max number of CPUs
#define UNUSED(x) (void)(x)

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  4

extern uint64 global_tick;

#endif
