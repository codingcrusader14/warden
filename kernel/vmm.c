#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include "libk/includes/stdio.h"
#include "libk/includes/string.h"
#include "types.h"
#include "vmm.h"
#include "pmm.h"
#include "mmu_defs.h"

extern void tlb_invalidate(va_t virtual_address);

pte_t* kernel_L0;

void flush_tlb() {
  asm volatile("tlbi vmalle1is");
  asm volatile("dsb ish");
  asm volatile("isb");
}

void set_ttbr1_addr(uint64 addr) {
  asm volatile("msr ttbr1_el1, %0" :: "r"(addr));
  asm volatile("tlbi vmalle1is");
  asm volatile("dsb ish");
  asm volatile("isb");
} 

pte_t* alloc_page_table() {
  return (pte_t*) pmm_alloc();
}

static int is_table_free(pte_t* table) {
  for (size_t i = 0; i < TABLE_ENTRIES; ++i) {
    if (table[i]) { return 0; }
  }
  return 1;
}

int vmm_init() {
  kernel_L0 = alloc_page_table(); // kernel base page table
  if (!kernel_L0) { return -1; }

  pte_t* kernel_L0_kva = (pte_t*) PA_TO_KVA(kernel_L0);
  pte_t normal_mem = (GLOBAL | ACCESS | SH_INNER_SHAREABLE | AP_READ_WRITE | ATTRINDX(1) | PAGE_DESCRIPTOR | VALID);
  for (pa_t entry = (pa_t) KERNEL_MEMORY ; entry < (pa_t) QEMU_DRAM_START; entry += 0x1000) { // maps kernel image 
    if (map_page(kernel_L0_kva, PA_TO_KVA(entry), entry, normal_mem)) {
      return -1;
    }
  }
  
  pte_t device_mem = (GLOBAL | ACCESS | SH_NON_SHAREABLE | AP_READ_WRITE | ATTRINDX(0) | PAGE_DESCRIPTOR | VALID);  // maps kernel uart
  if (map_page(kernel_L0_kva, PA_TO_KVA(DEVICE_MEMORY), DEVICE_MEMORY, device_mem)) {
    return -1;
  }

  for (pa_t entry = (pa_t)VIRTIO_BASE; entry <= VIRTIO_END; entry += VIRTIO_STRIDE) { // maps virtio
    if (map_page(kernel_L0_kva, PA_TO_KVA(entry), entry, device_mem)) {
      return -1;
    }
  }

  for (pa_t entry = (pa_t) GICD ; entry < (pa_t) (GICC + 0x10000) ; entry += 0x1000) { // maps gic controller
    if (map_page(kernel_L0_kva, PA_TO_KVA(entry), entry, device_mem)) {
      return -1;
    }
  }

  for (pa_t entry = (pa_t) QEMU_DRAM_START ; entry < ((pa_t) (QEMU_DRAM_START + 0x2000000)); entry += 0x1000) { // allocates 2mb of kernel heap space in ram
    if (map_page(kernel_L0_kva, PA_TO_KVA(entry), entry, normal_mem)) {
      return -1;
    }
  }

  set_ttbr1_addr((uint64) kernel_L0);

  return 0;
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

  pte_t* L1_base_table = (pte_t*) PA_TO_KVA(base_table[L0_index] & TABLE_ADDR_MASK);
  va_t L1_index = (virtual_address >> TABLE_SHIFT(1)) & PAGE_BIT_ENTRIES;

  if (!L1_base_table[L1_index]) {
    pa_t* L1_tb_ptr = alloc_page_table();
    if (!L1_tb_ptr) {
      return -1;
    }
    L1_base_table[L1_index] = ((pte_t) (L1_tb_ptr) | TABLE_DESCRIPTOR | VALID);
  }

  pte_t* L2_base_table = (pte_t*) PA_TO_KVA(L1_base_table[L1_index] & TABLE_ADDR_MASK);
  va_t L2_index = (virtual_address >> TABLE_SHIFT(2)) & PAGE_BIT_ENTRIES;

  if (!L2_base_table[L2_index]) {
    pa_t* L2_tb_ptr = alloc_page_table();
    if (!L2_tb_ptr) {
      return -1;
    }
    L2_base_table[L2_index] = ((pte_t) (L2_tb_ptr) | TABLE_DESCRIPTOR | VALID);
  }
  
  pte_t* L3_base_table = (pte_t*) PA_TO_KVA(L2_base_table[L2_index] & TABLE_ADDR_MASK);
  va_t L3_index = (virtual_address >> TABLE_SHIFT(3)) & PAGE_BIT_ENTRIES;

  if (L3_base_table[L3_index] & VALID) {
    return -2; // entry is already mapped
  }
  L3_base_table[L3_index] = ((pte_t) physical_address | flags);
  return 0;
}

int unmap_page(pte_t* base_table, va_t virtual_address) {
  // clears the pte based off of the virtual address mapping, if the mapping 
  // does not exist it returns -1. 
  va_t L0_index = (virtual_address >> TABLE_SHIFT(0)) & PAGE_BIT_ENTRIES;
  if (!(base_table[L0_index] & VALID)) {
    return -1;
  }

  pte_t* L1_base_table = (pte_t*) PA_TO_KVA(base_table[L0_index] & TABLE_ADDR_MASK);
  va_t L1_index = (virtual_address >> TABLE_SHIFT(1)) & PAGE_BIT_ENTRIES;
  if (!(L1_base_table[L1_index] & VALID)) {
    return -1;
  }

  pte_t* L2_base_table = (pte_t*) PA_TO_KVA(L1_base_table[L1_index] & TABLE_ADDR_MASK);
  va_t L2_index = (virtual_address >> TABLE_SHIFT(2)) & PAGE_BIT_ENTRIES;
  if (!(L2_base_table[L2_index] & VALID)) {
    return -1;
  }

  pte_t* L3_base_table = (pte_t*) PA_TO_KVA(L2_base_table[L2_index] & TABLE_ADDR_MASK);
  va_t L3_index = (virtual_address >> TABLE_SHIFT(3)) & PAGE_BIT_ENTRIES;
  if (!(L3_base_table[L3_index] & VALID)) {
    return -1;
  }

  L3_base_table[L3_index] = 0; // clear pte
  tlb_invalidate(virtual_address >> PAGE_SHIFT); // invalidate tlb entry accross all cores
  // if the page table is empty corresponding to its level, then the underlying physical page is freed
  if (is_table_free(L3_base_table)) {
    L2_base_table[L2_index] = 0;
    pmm_free((pa_t*)KVA_TO_PA(L3_base_table));

    if (is_table_free(L2_base_table)) {
      L1_base_table[L1_index] = 0;
      pmm_free((pa_t*)KVA_TO_PA(L2_base_table));
  
      if (is_table_free(L1_base_table)) {
        base_table[L0_index] = 0;
        pmm_free((pa_t*)KVA_TO_PA(L1_base_table));
      }
    }
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

    pte_t* L1_base_table = (pte_t*)PA_TO_KVA(base_table[L0_index] & TABLE_ADDR_MASK);
    va_t L1_index = (virtual_address >> TABLE_SHIFT(1)) & PAGE_BIT_ENTRIES;
    if (!L1_base_table[L1_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L1 Page Table Index: %d -- L1 Page Table Entry: %p\n", L1_index, L1_base_table[L1_index]);

    pte_t* L2_base_table = (pte_t*)PA_TO_KVA(L1_base_table[L1_index] & TABLE_ADDR_MASK);
    va_t L2_index = (virtual_address >> TABLE_SHIFT(2)) & PAGE_BIT_ENTRIES;
    if (!L2_base_table[L2_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L2 Page Table Index: %d -- L2 Page Table Entry: %p\n", L2_index, L2_base_table[L2_index]);

    pte_t* L3_base_table = (pte_t*)PA_TO_KVA(L2_base_table[L2_index] & TABLE_ADDR_MASK);
    va_t L3_index = (virtual_address >> TABLE_SHIFT(3)) & PAGE_BIT_ENTRIES;
    if (!L3_base_table[L3_index]) {
      kprintf("UNMAPPED ERROR\n");
      return;
    }
    kprintf("L3 Page Table Index: %d -- L3 Page Table Entry: %p\n", L3_index, L3_base_table[L3_index]);
  }

static pte_t* walk(pte_t* base, va_t va) {
  size_t l0 = (va >> TABLE_SHIFT(0)) & PAGE_BIT_ENTRIES;
  if (!(base[l0] & VALID)) return NULL;

  pte_t* l1 = (pte_t*)PA_TO_KVA(base[l0] & TABLE_ADDR_MASK);
  size_t l1_index = (va >> TABLE_SHIFT(1)) & PAGE_BIT_ENTRIES;
  if (!(l1[l1_index] & VALID)) return NULL;

  pte_t* l2 = (pte_t*)PA_TO_KVA(l1[l1_index] & TABLE_ADDR_MASK);
  size_t l2_index = (va >> TABLE_SHIFT(2)) & PAGE_BIT_ENTRIES;
  if (!(l2[l2_index] & VALID)) return NULL;

  pte_t* l3 = (pte_t*)PA_TO_KVA(l2[l2_index] & TABLE_ADDR_MASK);
  size_t l3_index = (va >> TABLE_SHIFT(3)) & PAGE_BIT_ENTRIES;
  if (!(l3[l3_index] & VALID)) return NULL;

  return &l3[l3_index];
}

int copy_to_user(pte_t* user_pt, void* dst, const void* src, size_t len) {
    uintptr_t addr = (uintptr_t)dst;
    uintptr_t end = addr + len;
    uint64 copied = 0;

    if (end < addr) return -1;
    if (addr >= KERNEL_HIGH_VA_ADDRESS) return -1;

    while (copied < len) {
        pte_t* pte = walk(user_pt, addr);
        if (!pte) return -1;

        pa_t pa = (*pte & TABLE_ADDR_MASK) + (addr & PAGE_BITS);
        va_t kva = PA_TO_KVA(pa);

        uint64 page_remaining = PAGE_SIZE - (addr & PAGE_BITS);
        uint64 chunk = (page_remaining < (len - copied)) ? page_remaining : (len - copied);

        memcpy((void*)kva, src + copied, chunk);

        copied += chunk;
        addr += chunk;
    }

    return 0;
}

int strncpy_from_user(pte_t* current_pt, const char* user_str, char* kernel_buf, size_t max) {
  size_t copied = 0;
  uintptr_t addr = (uintptr_t)user_str;

  while (copied < max) {
    size_t page_remaining = PAGE_SIZE - (addr & PAGE_BITS);
    size_t chunk = (page_remaining < (max - copied)) ? page_remaining : (max - copied);

    if (copy_from_user(current_pt, (const void*)addr, kernel_buf + copied, chunk) != 0) {
      return -1;
    }

    for (size_t i = 0; i< chunk; ++i) { // check for null terminator
      if (kernel_buf[copied + i] == '\0') 
        return copied + i;
    }

    copied += chunk;
    addr += chunk;
  }

  kernel_buf[max - 1] = '\0';
  return max - 1;
}

int copy_from_user(pte_t* current_pt,const void* user_buf, void* kernel_buf, size_t len) {
  uintptr_t addr = (uintptr_t) user_buf;
  uintptr_t end = addr + len;
  uint64 copied = 0;
  if (end < addr) return -1;
  if (addr >= KERNEL_HIGH_VA_ADDRESS) return -1;

  while (copied < len) {
    pte_t* pte = walk(current_pt, addr);
    if (!pte) return -1;

    pa_t physical_address = (*pte & TABLE_ADDR_MASK) + (addr & PAGE_BITS);

    va_t kvirtual_address = PA_TO_KVA(physical_address);

    uint64 page_remaining = PAGE_SIZE - (addr & PAGE_BITS);

    uint64 chunk = (page_remaining < (len - copied)) ? page_remaining : (len - copied);

    memcpy(kernel_buf + copied, (const void*) kvirtual_address, chunk);
    
    copied += chunk;
    addr += chunk;
  }

  return 0;
}

int copy_user_pagetable(pte_t* parent_pgd, pte_t* child_pgd) {
  pte_t pt_rows = (PAGE_SIZE / PTE_SIZE);

  for (va_t l0_index = 0; l0_index < pt_rows; ++l0_index) {
    if (!(parent_pgd[l0_index] & VALID)) continue;
    pte_t* l0_table = (pte_t*)PA_TO_KVA(parent_pgd[l0_index] & TABLE_ADDR_MASK);

    for (va_t l1_index = 0; l1_index < pt_rows; ++l1_index) {
      if (!(l0_table[l1_index] & VALID)) continue;
      pte_t* l1_table = (pte_t*)PA_TO_KVA(l0_table[l1_index] & TABLE_ADDR_MASK);

      for (va_t l2_index = 0; l2_index < pt_rows; ++l2_index) {
        if (!(l1_table[l2_index] & VALID)) continue;
        pte_t* l2_table = (pte_t*)PA_TO_KVA(l1_table[l2_index] & TABLE_ADDR_MASK);

        for (va_t l3_index = 0; l3_index < pt_rows; ++l3_index) {
          if (!(l2_table[l3_index] & VALID)) continue;

          pte_t entry = l2_table[l3_index];
          pa_t physical_address = entry & TABLE_ADDR_MASK;
          uint64 flags = entry & ~TABLE_ADDR_MASK;
          pte_t* child_page = pmm_alloc();
          if (!child_page) {
            kprintf("Child page failed to allocate\n");
            return -1;
          }
          memcpy((void*)PA_TO_KVA(child_page),(void*)PA_TO_KVA(physical_address), PAGE_SIZE);
          va_t va = (l0_index << TABLE_SHIFT(0)) | (l1_index << TABLE_SHIFT(1)) | (l2_index << TABLE_SHIFT(2)) | (l3_index << TABLE_SHIFT(3));
          int code = map_page(child_pgd, va, (pa_t) child_page, flags);
          if (code < 0) {
            kprintf("Failed to map page\n");
            return -1;
          }
        }
      }
    }
  }
  return 0;
}

void free_user_pages(pte_t* pgd) {
  pte_t pt_rows = (PAGE_SIZE / PTE_SIZE);

  for (va_t l0_index = 0; l0_index < pt_rows; ++l0_index) {
    if (!(pgd[l0_index] & VALID)) continue;
    pte_t* l0_table = (pte_t*)PA_TO_KVA(pgd[l0_index] & TABLE_ADDR_MASK);

    for (va_t l1_index = 0; l1_index < pt_rows; ++l1_index) {
      if (!(l0_table[l1_index] & VALID)) continue;
      pte_t* l1_table = (pte_t*)PA_TO_KVA(l0_table[l1_index] & TABLE_ADDR_MASK);

      for (va_t l2_index = 0; l2_index < pt_rows; ++l2_index) {
        if (!(l1_table[l2_index] & VALID)) continue;
        pte_t* l2_table = (pte_t*)PA_TO_KVA(l1_table[l2_index] & TABLE_ADDR_MASK);

        for (va_t l3_index = 0; l3_index < pt_rows; ++l3_index) {
          if (!(l2_table[l3_index] & VALID)) continue;

          pa_t page = (pa_t)(l2_table[l3_index] & TABLE_ADDR_MASK);
          pmm_free((pa_t*)page);
          l2_table[l3_index] = 0;
        }
        pmm_free((pa_t*)KVA_TO_PA(l2_table));
        l1_table[l2_index] = 0;
      }
      pmm_free((pa_t*)KVA_TO_PA(l1_table));
      l0_table[l1_index] = 0;
    }
    pmm_free((pa_t*)KVA_TO_PA(l0_table));
    pgd[l0_index] = 0;
  }
}
