#ifndef PMM_H
#define PMM_H

#include "types.h"

extern char _kernel_end;
#define QEMU_DRAM_START (pa_t*)&_kernel_end
#define QEMU_DRAM_END (pa_t*)0x60000000UL

void pmm_init(pa_t* start, pa_t* end);
pa_t* pmm_alloc();
void pmm_free(pa_t* address);

#endif
