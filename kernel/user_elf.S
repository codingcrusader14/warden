.section .rodata
.global _user_elf_start
.global _user_elf_end

_user_elf_start:
  .incbin "build/user/init.elf"
_user_elf_end:
