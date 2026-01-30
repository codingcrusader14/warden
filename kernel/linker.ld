OUTPUT_ARCH ( "AArch64" )
ENTRY( _entry ) /* bootloader will look at this image and start execution at the symbol designated as entry point */

PHDRS {
  text PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data PT_LOAD FLAGS(6);
}


SECTIONS
{
  . = 0x40080000;

  .text : {
      KEEP(*(.text.boot))
      *(.text)
  } : text

  .vector_table : {
    . = ALIGN(0x800);
    vtable_start = .;
    KEEP(*(.vector_table))
    . = . + 0x800; /* 2KB vector table */
  }

    .rodata : {
    *(.rodata)
  } : rodata
    
  .data : {
    *(.data)
  } : data

  .bss : {
    *(.bss)
  }

  . = ALIGN(16);
  . = . + 0x4000; /* 16KB stack */
  stack_top = .;
}


