#include <plat_def.h>

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY (_start)
MEMORY {
    ROM(rx): ORIGIN = 0, LENGTH = 0x20000
    SRAM (rw): ORIGIN = 0x0e000000, LENGTH = 1M
    DDR (rwx): ORIGIN = MEMORY_USE_BADDR, LENGTH = MEMORY_USER_SIZE
} 
SECTIONS
{
    . = ORIGIN(ROM); /* for virt board */
    image_start = .;
    .text : {
        *(.text.boot)
        *(.text)
        *(.text*)
        . = ALIGN(4K);
        *(.vectors)
    } > ROM
    .rodata : { 
        *(.rodata)
    } > ROM
    
    . = ALIGN(8);
    __data_rw_start = .;
    .data : {
        _d_copy_start = .; /*or use LOADADDR(.data)*/
        *(.data)
        *(.data*)
    } > SRAM AT > ROM
   
    . = ALIGN(8);
    .got :{
        *(.got*)
        _d_copy_end = .;
    } > SRAM AT > ROM

    . = ALIGN(8);
    .rela.dyn : {
     __rel_dyn_start = .;
    *(.rela*)
    __rel_dyn_end = .;
    } > SRAM

    . = ALIGN(8);
    image_end = .;
    bss_begin = .;
    .bss (NOLOAD) : {
        *(.bss)
        *(.bss*)
    } > SRAM
    bss_end = .;
    .ddr_data (NOLOAD) : {
        . = ALIGN(4096);
        *(.ddr_data*)
    } > DDR
}

