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
        *(.page_data*)
        . = ALIGN(8);
        ddr_data_start = .;
        *(.ddr_data*)
    } > DDR
}

