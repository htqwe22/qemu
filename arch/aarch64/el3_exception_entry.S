#include <asm_macros.S>
#include <arch.h>


.globl	el3_exception_table

/*
 * 异常处理需要保存相关冲突的寄存器（常见的是x0-x15）
 * x0 - x7是正常调用的参数，一般正常调用时被调用者可以直接使用。
 * 正常函数调用 x9-x15是可以随意使用，调用者会保存（调用者如果没有使用，可以不用管）
 * 正常函数调用 x19-x28调用者不会保存，如果被调用者要使用，也要先压栈保存（不使用也不用管）
 * 
 */

vector_base el3_exception_table

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
	stp x0, x1, [sp, #-16]!
	mov	x0, #0
	mov x1, sp
	bl	el3_sync_handler
	ldp x0, x1, [sp], #16
	eret
end_vector_entry SynchronousExceptionSP0

vector_entry IrqSP0
	mov	x0, #0x80
	mov x1, sp
	bl	el3_irq_handler
    eret
end_vector_entry IrqSP0

vector_entry FiqSP0
	mov	x0, #0x100
	mov x1, sp
	bl	el3_fiq_handler
    eret
end_vector_entry FiqSP0

vector_entry SErrorSP0
	mov	x0, #0x180
	mov x1, sp
	bl	el3_serror_handler
    eret
end_vector_entry SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
	mov	x0, #0x200
	mov x1, sp
	bl	el3_sync_handler
	eret
end_vector_entry SynchronousExceptionSPx

vector_entry IrqSPx
	mov	x0, #0x280
	mov x1, sp
	bl	el3_irq_handler
	eret
end_vector_entry IrqSPx

vector_entry FiqSPx
	mov	x0, #0x300
	bl	el3_fiq_handler
	eret
end_vector_entry FiqSPx

vector_entry SErrorSPx
	mov	x0, #0x380
	bl	el3_serror_handler
	eret
end_vector_entry SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	mov	x0, #0x400
	bl	el3_sync_handler
	eret
end_vector_entry SynchronousExceptionA64

vector_entry IrqA64
	mov	x0, #0x480
	bl	el3_irq_handler
	eret
end_vector_entry IrqA64

vector_entry FiqA64
	mov	x0, #0x500
	bl	el3_fiq_handler
	eret
end_vector_entry FiqA64

vector_entry SErrorA64
	mov	x0, #0x580
	bl	el3_serror_handler
	eret
end_vector_entry SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32
	mov	x0, #0x600
	bl	el3_sync_handler
	eret
end_vector_entry SynchronousExceptionA32

vector_entry IrqA32
	mov	x0, #0x680
	bl	el3_irq_handler
	eret
end_vector_entry IrqA32

vector_entry FiqA32
	mov	x0, #0x700
	bl	el3_fiq_handler
	eret
end_vector_entry FiqA32

vector_entry SErrorA32
	mov	x0, #0x780
	bl	el3_serror_handler
	eret
end_vector_entry SErrorA32