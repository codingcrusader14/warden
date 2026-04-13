#include "elf.h"
#include "mmu_defs.h"
#include "vmm.h"
#include "pmm.h"
#include "libk/includes/string.h"
#include "libk/includes/stdio.h"

static bool elf_check_supported(const ELFHeader* hdr) {
  if (!hdr) { return false; }
  static const unsigned char elf_mag[] = {0x7F, 'E', 'L', 'F'};

  if (memcmp(hdr->e_ident, elf_mag, sizeof(elf_mag)) != 0) {
    kprintf("ELF: bad magic\n");
    return false;
  }

  if (hdr->e_ident[EL_CLASS] != ELFCLASS64) {
    kprintf("ELF: not 64-bit\n");
    return false;
  }

  if (hdr->e_ident[EL_DATA] != ELFDATA2LSB) {
    kprintf("ELF: not little-endian\n");
    return false;
  }

  if (hdr->e_type != ET_EXEC) {
    kprintf("ELF: not executable\n");
    return false;
  }

  if (hdr->e_machine != EM_AARCH64) {
    kprintf("ELF: not AArch64\n");
    return false;
  }
  return true;
}

uint64 parse_and_map_elf(const void* elf_buf, size_t len, task_t* proc) {
  if (!elf_buf) return 0; 

  pte_t* pgd_kva = (pte_t*) PA_TO_KVA(proc->pgd);

  ELFHeader elf; 
  memcpy(&elf, elf_buf, sizeof(ELFHeader));
  if (!elf_check_supported(&elf)) 
    return 0;

  if (elf.e_phentsize != sizeof(ProgramHeader64))
     return 0;
  
  if (elf.e_phoff + (elf.e_phentsize * elf.e_phnum) > len)
    return 0;

  const void* program_hdr_table = elf_buf + elf.e_phoff;
  size_t code_size = 0;
  for (size_t i = 0; i < elf.e_phnum; ++i) {
    ProgramHeader64 segment;
    memcpy(&segment, program_hdr_table + (i * elf.e_phentsize), sizeof(ProgramHeader64));
    if (segment.type == PT_LOAD) {
      if (segment.offset + segment.filesz > len) 
        return 0;
      
      uint64 seg_end = segment.vaddr + segment.memsz;
      if (seg_end > code_size) {
        code_size = seg_end;
      }
      uint64 num_pages = (segment.memsz + PAGE_SIZE - 1) / PAGE_SIZE;
      for (uint64 j = 0; j < num_pages; ++j) {
        pa_t page = (pa_t)pmm_alloc();
        map_page(pgd_kva, segment.vaddr + (j * PAGE_SIZE), page, USER_FLAGS);

        uint64 offset = j * PAGE_SIZE;
        if (offset >= segment.filesz) {
          continue;
        }
        uint64 copy_size = (segment.filesz - offset > PAGE_SIZE) ? PAGE_SIZE : segment.filesz - offset;
        memcpy((void*)PA_TO_KVA(page), (const char*) elf_buf + segment.offset + offset, copy_size);
      }
    }
  }
  proc->brk = (code_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
  proc->code_size = code_size;
  return elf.e_entry;
}
