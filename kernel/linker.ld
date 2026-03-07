OUTPUT_ARCH ( "AArch64" )
ENTRY( _entry ) /* bootloader will look at this image and start execution at the symbol designated as entry point */
KERNEL_VA_OFFSET = 0xFFFF000000000000; /* kernel lives in upper va space */
_qemu_kernel_physical_base = 0x40080000;

PHDRS {
  boot PT_LOAD FLAGS(5);
  text PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data PT_LOAD FLAGS(6);
}

SECTIONS
{
  . = _qemu_kernel_physical_base;

  .text.boot : {

    KEEP(*(.text.boot))
  } : boot 

  . = ALIGN(0x1000);
  boot_pagetables = .;

  boot_ttbr1_l0 = .;
  . = . + 0x1000;
  boot_ttbr1_l1 = .;

  . = . + 0x1000;
  boot_ttbr0_l0 = .;

  . = . + 0x1000;
  boot_ttbr0_l1 = .;


  . = . + 0x1000;
  _boot_end = .;
  . = KERNEL_VA_OFFSET + _boot_end;

  _boot_high = .;
  .text : AT(_boot_end) {
    *(.text)
    . = ALIGN(0x800);
    vtable_start = .;
    KEEP(*(.vector_table))
    . = . + 0x800; /* 2KB vector table */
  } : text 

  .rodata : AT(ADDR(.rodata) - KERNEL_VA_OFFSET) {
    *(.rodata)
  } : rodata
  
  .data : AT(ADDR(.data) - KERNEL_VA_OFFSET){
    *(.data)
  } : data

  .bss : AT(ADDR(.bss) - KERNEL_VA_OFFSET) {
    _bss_start = .;
    *(.bss)
    _bss_end = .;
  }

  . = ALIGN(16);
  . = . + 0x4000; /* 16KB stack */
  stack_top = .;

  . = ALIGN(4096);
  _qemu_kernel_end = .;
  _qemu_physical_end = _qemu_kernel_end - KERNEL_VA_OFFSET;
}


