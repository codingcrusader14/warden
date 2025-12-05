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


