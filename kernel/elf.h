#ifndef ELF_H
#define ELF_H

#include "types.h"
#include "process.h"

#define EI_NIDENT 16

#define ELFDATA2LSB (1) // Little Endian AArch64
#define ELFCLASS64 (2) // 64-bit Architecture
#define ET_EXEC (2) // Executable file
#define EM_AARCH64 (0xB7) // ARMv8

#define PT_LOAD (1) // Loadable segment

typedef uint16 halfword; 
typedef uint32 word;
typedef uint64 address;
typedef uint64 offset;
typedef uint64 doubleword;

typedef struct {
  unsigned char e_ident[EI_NIDENT]; // how elf file needs to be parsed
  halfword e_type; // type of elf file
  halfword e_machine; // type of machine x86, arm, ect..
  word e_version; // always set to 1
  address e_entry; // entry point for our executable
  offset e_phoff; // program header offset relative to start of file
  offset e_shoff; // section header offset relative to start of file
  word e_flags; // os dependent 
  halfword e_ehsize; // size of this elf header
  halfword e_phentsize; // size of program headers
  halfword e_phnum; // number of program headers
  halfword e_shentsize; // size of section headers
  halfword e_shnum; // number of section headers
  halfword e_shstrndx; // section header string table
} ELFHeader;

enum ELF_ident {
  EL_MAG0 = 0, // 0x7F
  EL_MAG1 = 1, // 'E'
  EL_MAG2 = 2, // 'L'
  EL_MAG3 = 3, // 'F'
  EL_CLASS = 4, // Arch (32/64)
  EL_DATA = 5, // Endianess
  EL_VERSION = 6, // ELF Version
  EL_OSABI = 7, // OS Specific
  EL_ABIVERSION = 8, // OS Specific
  EL_PAD = 9, // Padding
};

typedef struct {
  word type; // type of segment
  word flags; // permission of the segment
  offset offset; // offset of this particular segment
  address vaddr; // virtual address of where first byte is located 
  address paddr; // physical address of where first byte is located, used in firmware
  doubleword filesz; // the size of the segment in the specified file
  doubleword memsz; // size of the segment in memory 
  doubleword align; // tells alignment of segment (4 or 8 bytes)
} ProgramHeader64; 

uint64 parse_and_map_elf(const void* elf_buf, size_t len, task_t* proc);



#endif
