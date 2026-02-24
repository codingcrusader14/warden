#ifndef VMM_H
#define VMM_H

#include "types.h"

#ifndef __ASSEMBLER__
  int vmm_init();
#endif
extern pte_t* kernel_L0;
pte_t* alloc_page_table();
int map_page(pte_t* base_table, va_t virtual_address, pa_t physical_address, pte_t flags);
void debug_va(pte_t* base_table, va_t virtual_address);


#endif 
