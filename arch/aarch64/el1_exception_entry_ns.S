#include <asm_macros.S>
#include <arch.h>


.globl	el1_ns_exception_table

vector_base el1_ns_exception_table

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
	save_caller_regs el1
	mov	x0, #0
	bl	el1_ns_sync_handler
	restore_caller_regs el1
	eret
end_vector_entry SynchronousExceptionSP0

vector_entry IrqSP0
	save_caller_regs el1
	mov	x0, #0x80
	bl	el1_ns_irq_handler
	restore_caller_regs el1
    eret
end_vector_entry IrqSP0

vector_entry FiqSP0
	save_caller_regs el1
	mov	x0, #0x100
	bl	el1_ns_fiq_handler
	restore_caller_regs el1
    eret
end_vector_entry FiqSP0

vector_entry SErrorSP0
	save_caller_regs el1
	mov	x0, #0x180
	bl	el1_ns_serror_handler
	restore_caller_regs el1
    eret
end_vector_entry SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
	save_caller_regs el1
	mov	x0, #0x200
	bl	el1_ns_sync_handler
	restore_caller_regs el1
	eret
end_vector_entry SynchronousExceptionSPx

vector_entry IrqSPx
	save_caller_regs el1
	mov	x0, #0x280
	bl	el1_ns_irq_handler
	restore_caller_regs el1
	eret
end_vector_entry IrqSPx

vector_entry FiqSPx
	save_caller_regs el1
	mov	x0, #0x300
	bl	el1_ns_fiq_handler
	restore_caller_regs el1
	eret
end_vector_entry FiqSPx

vector_entry SErrorSPx
	save_caller_regs el1
	mov	x0, #0x380
	bl	el1_ns_serror_handler
	restore_caller_regs el1
	eret
end_vector_entry SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	save_caller_regs el1
	mov	x0, #0x400
	bl	el1_ns_sync_handler
	restore_caller_regs el1
	eret
end_vector_entry SynchronousExceptionA64

vector_entry IrqA64
	save_caller_regs el1
	mov	x0, #0x480
	bl	el1_ns_irq_handler
	restore_caller_regs el1
	eret
end_vector_entry IrqA64

vector_entry FiqA64
	save_caller_regs el1
	mov	x0, #0x500
	bl	el1_ns_fiq_handler
	restore_caller_regs el1
	eret
end_vector_entry FiqA64

vector_entry SErrorA64
	save_caller_regs el1
	mov	x0, #0x580
	bl	el1_ns_serror_handler
	restore_caller_regs el1
	eret
end_vector_entry SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32
	mov	x0, #0x600
	bl	el1_ns_sync_handler
	eret
end_vector_entry SynchronousExceptionA32

vector_entry IrqA32
	mov	x0, #0x680
	bl	el1_ns_irq_handler
	eret
end_vector_entry IrqA32

vector_entry FiqA32
	mov	x0, #0x700
	bl	el1_ns_fiq_handler
	eret
end_vector_entry FiqA32

vector_entry SErrorA32
	mov	x0, #0x780
	bl	el1_ns_serror_handler
	eret
end_vector_entry SErrorA32