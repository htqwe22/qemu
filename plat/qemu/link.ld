OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY (_start)
/* MEMORY {
    DDR (rwx): ORIGIN = 0x40000000, LENGTH = 1024M
    SRAM (rw): ORIGIN = 0x0e000000, LENGTH = 0x00100000
} */
SECTIONS
{
    /* . = 0x80000; */
    . = 0x00000000; /* for virt board */
    image_start = .;
    .text.boot : {*(.text.boot)}
    .text : {*(.text)}
    . = ALIGN(11);
    .vectors :{*(.vectors)}
    . = ALIGN(8);
    .rodata : { *(.rodata)}
    .data : {*(.data)}

    . = ALIGN(8);
    .rela.dyn : {
     __rel_dyn_start = .;
    *(.rela*)
    __rel_dyn_end = .;
    }
    . = ALIGN(8);
    image_end = .;
    bss_begin = .;
    .bss : {*(.bss*)}
    bss_end = .;
    /* 
    .ddr_data (NOLOAD) : {
        __DDR_START__ = .;
        *(.ddr_data)
         __DDR_END__ = .;
    } > DDR */
}

