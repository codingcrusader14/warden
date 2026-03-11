ENTRY(_start) 

PAGE_SIZE = 4096;
USER_BASE = 0x400000;

SECTIONS
{
  . = USER_BASE;

  .text : {
    *(.text .text.*)
  }

  . = ALIGN(PAGE_SIZE);
  .rodata : {
    *(.rodata .rodata.*)
  }

  . = ALIGN(PAGE_SIZE);
  .data : {
    *(.data .data.*)
  }

  . = ALIGN(PAGE_SIZE);
  .bss : {
    __bss_start = .;
    *(.bss .bss.* COMMON)
    __bss_end = .;
  }

  . = ALIGN(PAGE_SIZE);
  _end = .;

  /DISCARD/ : {
    *(.comment .note.* .eh_frame*)
  }
}
