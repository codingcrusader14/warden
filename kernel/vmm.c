#include "vmm.h"
#include "types.h"
#include <stdalign.h>

extern void configure_mmu(pte_t* tt_base_register);

alignas(4096) static pte_t L0[512];
alignas(4096) static pte_t L1[512]; 
alignas(4096) static pte_t L2_DEVICE[512];
alignas(4096) static pte_t L2_NORMAL[512];

void vmm_init() {
  L0[0] = ((pte_t)L1 | TABLE_DESCRIPTOR | VALID); 
  L1[0] = ((pte_t)L2_DEVICE | TABLE_DESCRIPTOR | VALID);
  L1[1] = ((pte_t)L2_NORMAL | TABLE_DESCRIPTOR | VALID);

  L2_DEVICE[72] = ((pte_t)DEVICE_MEMORY | GLOBAL | ACCESS | SH_NON_SHAREABLE | AP_READ_WRITE | ATTRINDX(0) | BLOCK_DESCRIPTOR | VALID);

  for (pte_t entry = 0; entry < 256; ++entry) {
    L2_NORMAL[entry] = ((pte_t)(KERNEL_MEMORY + (entry * 0x200000)) | GLOBAL | ACCESS | SH_INNER_SHAREABLE | AP_READ_WRITE | ATTRINDX(1) | BLOCK_DESCRIPTOR | VALID); // increment by 2MB per pte
  }

  configure_mmu(L0);
}
