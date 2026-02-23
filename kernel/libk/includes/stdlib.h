#ifndef LIBK_STDLIB_H
#define LIBK_STDLIB_H

#include "../../types.h"
#include <stdbool.h>
#include <stdalign.h>

typedef struct block_header_t {
  uint32 size;
  bool free;
  struct block_header_t* next;
} __attribute__((aligned(16))) block_header_t;

void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
