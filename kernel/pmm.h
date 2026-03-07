#ifndef PMM_H
#define PMM_H

#include "types.h"
#include "mmu_defs.h"

extern char _qemu_kernel_end;
#define QEMU_DRAM_START (pa_t*) KVA_TO_PA(&_qemu_kernel_end)
#define QEMU_DRAM_END (pa_t*)0x60000000UL

void print_head();
void pmm_init(pa_t* start, pa_t* end);
pa_t* pmm_alloc();
void pmm_free(pa_t* address);

#endif
