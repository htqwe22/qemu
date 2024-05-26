
.section ".text.boot"

.global _start
_start:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
    b proc_hang


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

    mov sp, #0x2000000 
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


