
.section ".text.boot"

.global _start
_start:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
    
proc_hang:
    wfe
    b proc_hang

master:    
    // clean bss ...
    ldr x0, =bss_begin
    mov x1, xzr
    ldr x2, =bss_end
    sub x2, x2, x0
    bl memset_64

//    mrs x3, sctlr_el1
//    bic x3, x3, #8
//    msr sctlr_el1, x3

    // disable mmu for el3
    bl disable_mmu_el3

    // set SP before go to C functions.
    mov sp, #0xC0000000 // for virt board point at the EOF 2G ddr

    bl main

    b proc_hang
    ret

.global memset_64
memset_64:
    mov x3, x0
    cmp x2, 0
    b.eq    2f
1:
    str  xzr, [x0], #8
    subs x2, x2, #8
    b.cc 1b     //no overflow again ...
2:
    mov x0, x3
    ret


.global memcpy_64
memcpy_64:

    ret

.global asm_test
asm_test :
    ldp x1, x2, [x0, #0x10]!

    ret
