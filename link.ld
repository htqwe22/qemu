OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY (_start)

SECTIONS
{
    /* . = 0x80000; */
    . = 0x40000000; /* for virt board */
    .text.boot : {*(.text.boot)}
    .text : {*(.text)}
    . = ALIGN(8);
    .rodata : { *(.rodata)}
    .data : {*(.data)}
    . = ALIGN(8);
    bss_begin = .;
    .bss : {*(.bss*)}
    bss_end = .;
}

