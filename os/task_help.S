#include <arch.h>
#define GENERAL_REG_OFFSET  0x20

.section ".text.asm.os"
.align 8


.macro save_task_context eln
    // here x0 x1 is not real ...
    // stp x0, x1,     [x0, #(GENERAL_REG_OFFSET)]
    mov x1, sp
    str x1,         [x0] //save sp ...
    stp x2, x3,     [x0, #(GENERAL_REG_OFFSET + 0x10)]
    stp x4, x5,     [x0, #(GENERAL_REG_OFFSET + 0x20)]
    stp x6, x7,     [x0, #(GENERAL_REG_OFFSET + 0x30)]
    stp x8, x9,     [x0, #(GENERAL_REG_OFFSET + 0x40)] 
    stp x10, x11,   [x0, #(GENERAL_REG_OFFSET + 0x50)]
    stp x12, x13,   [x0, #(GENERAL_REG_OFFSET + 0x60)]
    stp x14, x15,   [x0, #(GENERAL_REG_OFFSET + 0x70)]
    stp x16, x17,   [x0, #(GENERAL_REG_OFFSET + 0x80)] 
    stp x18, x19,   [x0, #(GENERAL_REG_OFFSET + 0x90)] 
    stp x20, x21,   [x0, #(GENERAL_REG_OFFSET + 0xa0)] 
    stp x22, x23,   [x0, #(GENERAL_REG_OFFSET + 0xb0)] 
    stp x24, x25,   [x0, #(GENERAL_REG_OFFSET + 0xc0)] 
    stp x26, x27,   [x0, #(GENERAL_REG_OFFSET + 0xd0)] 
    stp x28, x29,   [x0, #(GENERAL_REG_OFFSET + 0xe0)]
    str x30,        [x0, #(GENERAL_REG_OFFSET + 0xf0)]
    mrs x2, elr_\eln
    mrs x3, spsr_\eln
    stp x2,  x3,     [x0, #8]
.endm

.macro restore_task_context eln
    str x1, [x0]
    mov sp, x1      //restore sp ...
    ldp x2, x3,     [x0, #8]
    msr elr_\eln, x2
    msr spsr_\eln, x3
    ldp x2, x3,     [x0, #(GENERAL_REG_OFFSET + 0x10)]
    ldp x4, x5,     [x0, #(GENERAL_REG_OFFSET + 0x20)]
    ldp x6, x7,     [x0, #(GENERAL_REG_OFFSET + 0x30)]
    ldp x8, x9,     [x0, #(GENERAL_REG_OFFSET + 0x40)] 
    ldp x10, x11,   [x0, #(GENERAL_REG_OFFSET + 0x50)]
    ldp x12, x13,   [x0, #(GENERAL_REG_OFFSET + 0x60)]
    ldp x14, x15,   [x0, #(GENERAL_REG_OFFSET + 0x70)]
    ldp x16, x17,   [x0, #(GENERAL_REG_OFFSET + 0x80)] 
    ldp x18, x19,   [x0, #(GENERAL_REG_OFFSET + 0x90)] 
    ldp x20, x21,   [x0, #(GENERAL_REG_OFFSET + 0xa0)] 
    ldp x22, x23,   [x0, #(GENERAL_REG_OFFSET + 0xb0)] 
    ldp x24, x25,   [x0, #(GENERAL_REG_OFFSET + 0xc0)] 
    ldp x26, x27,   [x0, #(GENERAL_REG_OFFSET + 0xd0)] 
    ldp x28, x29,   [x0, #(GENERAL_REG_OFFSET + 0xe0)]
    ldr x30,        [x0, #(GENERAL_REG_OFFSET + 0xf0)]
.endm



task_context_switch:
    // save real x0, x1
    stp x0, x1, [sp, #-16]!
    ldr x1, =g_current_tcb
    ldr x0, [x1]    // get the value of g_current_tcb
    save_task_context el1
    ldp x0, x1, [sp], #16
    stp x0, x1,     [x0, #(GENERAL_REG_OFFSET)]
    dsb sy
    isb 
    
    bl fetch_next_run_task
    
    isb 
    // restore ...
    ldr x1, =g_current_tcb
    str x0, [x1]    // g_current_tcb = current tcb
    restore_task_context el1

    dsb sy
    isb 
    ret


    .global start_first_task
start_first_task:
    ldr x1, [x0]
    mov sp, x1
#if 1
    ldr x1, =g_current_tcb
    str x0, [x1]
    ldp x2, x3,    [x0, #(GENERAL_REG_OFFSET + 0x10)]
    ldp x4, x5,    [x0, #(GENERAL_REG_OFFSET + 0x20)]
    ldp x6, x7,    [x0, #(GENERAL_REG_OFFSET + 0x30)]
    ldp x8, x9,    [x0, #(GENERAL_REG_OFFSET + 0x40)] 
    ldp x10, x11,  [x0, #(GENERAL_REG_OFFSET + 0x50)]
    ldp x12, x13,  [x0, #(GENERAL_REG_OFFSET + 0x60)]
    ldp x14, x15,  [x0, #(GENERAL_REG_OFFSET + 0x70)]
    ldp x16, x17,  [x0, #(GENERAL_REG_OFFSET + 0x80)] 
    ldp x18, x19,  [x0, #(GENERAL_REG_OFFSET + 0x90)] 
    ldp x20, x21,  [x0, #(GENERAL_REG_OFFSET + 0xa0)] 
    ldp x22, x23,  [x0, #(GENERAL_REG_OFFSET + 0xb0)] 
    ldp x24, x25,  [x0, #(GENERAL_REG_OFFSET + 0xc0)] 
    ldp x26, x27,  [x0, #(GENERAL_REG_OFFSET + 0xd0)] 
    ldp x28, x29,  [x0, #(GENERAL_REG_OFFSET + 0xe0)]
#endif    
    ldp x30, xzr,  [x0, #0x110] 
    ldr x1, [x0, #0x28]
    ldr x0, [x0, #0x20]

    isb
    msr daifclr, #15
    dsb sy
    isb
    ret




