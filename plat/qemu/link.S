#include <plat_def.h>

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY (_start)
MEMORY {
    ROM(rx): ORIGIN = SEC_ROM_BASE, LENGTH = SEC_ROM_SIZE
    SRAM (rw): ORIGIN = SEC_SRAM_BASE, LENGTH = SEC_SRAM_SIZE - SEC_SRAM_SIZE *4
    DDR (rwx): ORIGIN = PLAT_DDR_BASE, LENGTH = PLAT_DDR_SIZE
} 
SECTIONS
{
    . = ORIGIN(ROM); /* for virt board */
    image_start = .;
    .text : {
        *start.o(.text.asm*)
        *(.text.asm*)
        . = ALIGN(2K);
        *(.vectors)
        /* *(.text.boot)       for golobal section setting*/
        *(.text)
        *(.text*)

    } > ROM

    .rodata : { 
        *(.rodata)
    } > ROM

    . = ALIGN(8);
    __data_lma_start = .; // or LOADADDR(.data)
    .data : {
        __data_link_start = .; /*or use LOADADDR(.data)*/
        *(.data)
        *(.data*)
    } > SRAM AT > ROM
   
    . = ALIGN(8);
    .got :{
        *(.got*)
        __data_link_end = .;
    } > SRAM AT > ROM

    . = ALIGN(8);
    .rela.dyn : {
     __rel_dyn_start = .;
    *(.rela*)
    __rel_dyn_end = .;
    } > SRAM
    image_end =  LOADADDR(.bss);
    . = ALIGN(8);
    bss_begin = .;
    .bss (NOLOAD) : {
        *(.bss)
        *(.bss*)
    } > SRAM
    bss_end = .;
    .ddr_data (NOLOAD) : {
        . = ALIGN(4096);
        *(.page_data*)
        . = ALIGN(8);
        ddr_data_start = .;
        *(.ddr_data*)
    } > DDR
}

