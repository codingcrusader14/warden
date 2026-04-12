#ifndef VMM_H
#define VMM_H

#include "types.h"

extern pte_t* kernel_L0;

#ifndef __ASSEMBLER__
  int vmm_init();
#endif

pte_t* alloc_page_table();
int map_page(pte_t* base_table, va_t virtual_address, pa_t physical_address, pte_t flags);
int unmap_page(pte_t* base_table, va_t virtual_address);
void debug_va(pte_t* base_table, va_t virtual_address);
int copy_to_user(pte_t* user_pt, void* dst, const void* src, size_t len);
int copy_from_user(pte_t* current_pt,const void* user_buf, void* kernel_buf, size_t len);
int strncpy_from_user(pte_t* current_pt, const char* user_str, char* kernel_buf, size_t max);
int copy_user_pagetable(pte_t* parent_pgd, pte_t* child_pgd);


#endif 
