#include "vmm.h"
#include "pmm.h"
#include "libk/includes/stdio.h"
#include <stdlib.h>
#include <stdalign.h>
#include "mmu_defs.h"

extern void configure_mmu(pte_t* user_tables, pte_t* kernel_tables);
pte_t* kernel_L0;

int vmm_init() {
  kernel_L0 = alloc_page_table(); // kernel base page table
  if (!kernel_L0) { return -1; }

  pte_t* user_L0 = alloc_page_table(); // user base page table
  if (!user_L0) { return -1; }

  pte_t normal_mem = (GLOBAL | ACCESS | SH_INNER_SHAREABLE | AP_READ_WRITE | ATTRINDX(1) | PAGE_DESCRIPTOR | VALID);
  for (pa_t entry = (pa_t) KERNEL_MEMORY ; entry < (pa_t) QEMU_DRAM_START; entry += 0x1000) { // maps kernel image 
    if (map_page(kernel_L0, entry, entry, normal_mem)) {
      return -1;
    }
  }
  
  pte_t device_mem =  (GLOBAL | ACCESS | SH_NON_SHAREABLE | AP_READ_WRITE | ATTRINDX(0) | PAGE_DESCRIPTOR | VALID); // maps kernel uart
  if (map_page(kernel_L0, DEVICE_MEMORY, DEVICE_MEMORY, device_mem)) {
    return -1;
  }

  for (pa_t entry = (pa_t) QEMU_DRAM_START ; entry < ((pa_t)QEMU_DRAM_START + 0x2000000); entry += 0x1000) { // allocates 2mb of kernel heap space in ram
    if (map_page(kernel_L0, entry, entry, normal_mem)) {
      return -1;
    }
  }

  configure_mmu(kernel_L0,user_L0);
  return 0;
}

pte_t* alloc_page_table() {
  return (pte_t*) pmm_alloc();
}

int map_page(pte_t* base_table, va_t virtual_address, pa_t physical_address, pte_t flags) {
  // AArch64 v8-a follows the following paging scheme for 4KB pages --- 
  // A 2^12 offset leaves us 2^36 to divide into 4 levels. Each page table
  // entry is 8 bytes and so 4KB / 8B = 512B which is 2^9. Each level then contains
  // 512 entries and so each level contains 2^9 bits of a specific mask of the virtual address
  // L0 - [47:39], L1 - [38:30], L2 - [29:21], L3 - [20:12], Offset - [11:0]
  va_t L0_index = (virtual_address >> TABLE_SHIFT(0)) & PAGE_BIT_ENTRIES;
  if (!base_table[L0_index]) {
    pa_t* base_tb_ptr = alloc_page_table();
    if (!base_tb_ptr) {
      return -1;
    }
    base_table[L0_index] = ((pte_t) (base_tb_ptr) | TABLE_DESCRIPTOR | VALID);
  }

  pte_t* L1_base_table = (pte_t*)(base_table[L0_index] & TABLE_ADDR_MASK);
  va_t L1_index = (virtual_address >> TABLE_SHIFT(1)) & PAGE_BIT_ENTRIES;

  if (!L1_base_table[L1_index]) {
    pa_t* L1_tb_ptr = alloc_page_table();
    if (!L1_tb_ptr) {
      return -1;
    }
    L1_base_table[L1_index] = ((pte_t) (L1_tb_ptr) | TABLE_DESCRIPTOR | VALID);
  }

  pte_t* L2_base_table = (pte_t*)(L1_base_table[L1_index] & TABLE_ADDR_MASK);
  va_t L2_index = (virtual_address >> TABLE_SHIFT(2)) & PAGE_BIT_ENTRIES;

  if (!L2_base_table[L2_index]) {
    pa_t* L2_tb_ptr = alloc_page_table();
    if (!L2_tb_ptr) {
      return -1;
    }
    L2_base_table[L2_index] = ((pte_t) (L2_tb_ptr) | TABLE_DESCRIPTOR | VALID);
  }
  
  pte_t* L3_base_table = (pte_t*) (L2_base_table[L2_index] & TABLE_ADDR_MASK);
  va_t L3_index = (virtual_address >> TABLE_SHIFT(3)) & PAGE_BIT_ENTRIES;

  if (!L3_base_table[L3_index]) {
    L3_base_table[L3_index] = ((pte_t) physical_address | flags);
  } else {
    return -2;
  }

  return 0;
}

void debug_va(pte_t* base_table, va_t virtual_address) {
    va_t L0_index = (virtual_address >> TABLE_SHIFT(0)) & PAGE_BIT_ENTRIES;
    if (!base_table[L0_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L0 Page Table Index: %d -- L0 Page Table Entry: %p\n", L0_index, base_table[L0_index]);

    pte_t* L1_base_table = (pte_t*)(base_table[L0_index] & TABLE_ADDR_MASK);
    va_t L1_index = (virtual_address >> TABLE_SHIFT(1)) & PAGE_BIT_ENTRIES;
    if (!L1_base_table[L1_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L1 Page Table Index: %d -- L1 Page Table Entry: %p\n", L1_index, L1_base_table[L1_index]);

    pte_t* L2_base_table = (pte_t*)(L1_base_table[L1_index] & TABLE_ADDR_MASK);
    va_t L2_index = (virtual_address >> TABLE_SHIFT(2)) & PAGE_BIT_ENTRIES;
    if (!L2_base_table[L2_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L2 Page Table Index: %d -- L2 Page Table Entry: %p\n", L2_index, L2_base_table[L2_index]);

    pte_t* L3_base_table = (pte_t*) (L2_base_table[L2_index] & TABLE_ADDR_MASK);
    va_t L3_index = (virtual_address >> TABLE_SHIFT(3)) & PAGE_BIT_ENTRIES;
    if (!L3_base_table[L3_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L3 Page Table Index: %d -- L3 Page Table Entry: %p\n", L3_index, L3_base_table[L3_index]);
  }

