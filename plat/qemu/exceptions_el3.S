#include <asm_macros.S>
#include <arch.h>


.globl	el3_exceptions


vector_base el3_exceptions

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
//	mov	x0, #SYNC_EXCEPTION_SP_EL0
	bl	plat_report_exception
	b	proc_hang
end_vector_entry SynchronousExceptionSP0

vector_entry IrqSP0
//	mov	x0, #IRQ_SP_EL0
	bl	plat_report_exception
    b	proc_hang
end_vector_entry IrqSP0

vector_entry FiqSP0
//	mov	x0, #FIQ_SP_EL0
	bl	plat_report_exception
    b	proc_hang
end_vector_entry FiqSP0

vector_entry SErrorSP0
//	mov	x0, #SERROR_SP_EL0
	bl	plat_report_exception
    b	proc_hang
end_vector_entry SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
//	mov	x0, #SYNC_EXCEPTION_SP_ELX
	bl	plat_report_exception
	b	proc_hang
end_vector_entry SynchronousExceptionSPx

vector_entry IrqSPx
//	mov	x0, #IRQ_SP_ELX
	bl	plat_report_exception
	b	proc_hang
end_vector_entry IrqSPx

vector_entry FiqSPx
//	mov	x0, #FIQ_SP_ELX
	bl	plat_report_exception
	b	proc_hang
end_vector_entry FiqSPx

vector_entry SErrorSPx
//	mov	x0, #SERROR_SP_ELX
	bl	plat_report_exception
	b	proc_hang
end_vector_entry SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	/* Enable the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

//	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/* Expect only SMC exceptions */
	mrs	x30, esr_el3
//	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH
//	cmp	x30, #EC_AARCH64_SMC
	b.ne	unexpected_sync_exception

	b	smc_handler64
end_vector_entry SynchronousExceptionA64

vector_entry IrqA64
//	mov	x0, #IRQ_AARCH64
	bl	plat_report_exception
	b	proc_hang
end_vector_entry IrqA64

vector_entry FiqA64
//	mov	x0, #FIQ_AARCH64
	bl	plat_report_exception
	b	proc_hang
end_vector_entry FiqA64

vector_entry SErrorA64
//	mov	x0, #SERROR_AARCH64
	bl	plat_report_exception
	b	proc_hang
end_vector_entry SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32
//	mov	x0, #SYNC_EXCEPTION_AARCH32
	bl	plat_report_exception
	b	proc_hang
end_vector_entry SynchronousExceptionA32

vector_entry IrqA32
//	mov	x0, #IRQ_AARCH32
	bl	plat_report_exception
	b	proc_hang
end_vector_entry IrqA32

vector_entry FiqA32
//	mov	x0, #FIQ_AARCH32
	bl	plat_report_exception
	b	proc_hang
end_vector_entry FiqA32

vector_entry SErrorA32
//	mov	x0, #SERROR_AARCH32
	bl	plat_report_exception
	b	proc_hang
end_vector_entry SErrorA32