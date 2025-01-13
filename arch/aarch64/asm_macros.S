	/*
	 * This macro is used to create a function label and place the
	 * code into a separate text section based on the function name
	 * to enable elimination of unused code during linking. It also adds
	 * basic debug information to enable call stack printing most of the
	 * time. The optional _align parameter can be used to force a
	 * non-standard alignment (indicated in powers of 2). The default is
	 * _align=2 because both Aarch32 and Aarch64 instructions must be
	 * word aligned. Do *not* try to use a raw .align directive. Since func
	 * switches to a new section, this would not have the desired effect.
	 */
	.macro func _name, _align=2
	/*
	 * Add Call Frame Information entry in the .debug_frame section for
	 * debugger consumption. This enables callstack printing in debuggers.
	 * This does not use any space in the final loaded binary, only in the
	 * ELF file.
	 * Note that a function manipulating the CFA pointer location (i.e. the
	 * x29 frame pointer on AArch64) should declare it using the
	 * appropriate .cfi* directives, or be prepared to have a degraded
	 * debugging experience.
	 */
	.cfi_sections .debug_frame
	.section .text.asm.\_name, "ax"
	.type \_name, %function
	/*
	 * .cfi_startproc and .cfi_endproc are needed to output entries in
	 * .debug_frame
	 */
	.cfi_startproc
	.align \_align
	\_name:
#if ENABLE_BTI
	/* When Branch Target Identification is enabled, insert "bti jc"
	 * instruction to enable indirect calls and branches
	 */
	 bti	jc
#endif
	.endm

	/*
	 * This macro is used to mark the end of a function.
	.macro endfunc _name
	.cfi_endproc
	.size \_name, . - \_name
	.endm
	 */
	 
	.macro endfunc _name
	.cfi_endproc
	.endm
	
	
	
	
	/*
        * Declare the exception vector table, enforcing it is aligned on a
        * 2KB boundary, as required by the ARMv8 architecture.
        * Use zero bytes as the fill value to be stored in the padding bytes
        * so that it inserts illegal AArch64 instructions. This increases
        * security, robustness and potentially facilitates debugging.
        */
    .macro vector_base  label, section_name=.vectors
    .section \section_name, "ax"
    .align 11, 0
    \label:
    .endm


	/*
	 * Create an entry in the exception vector table, enforcing it is
	 * aligned on a 128-byte boundary, as required by the ARMv8 architecture.
	 * Use zero bytes as the fill value to be stored in the padding bytes
	 * so that it inserts illegal AArch64 instructions. This increases
	 * security, robustness and potentially facilitates debugging.
	 */
	.macro vector_entry  label, section_name=.vectors
	.cfi_sections .debug_frame
	.section \section_name, "ax"
	.align 7, 0
	.type \label, %function
	.cfi_startproc
	\label:
	.endm

	/*
	 * Add the bytes until fill the full exception vector, whose size is always
	 * 32 instructions. If there are more than 32 instructions in the
	 * exception vector then an error is emitted.
	 */
	.macro end_vector_entry label
	.cfi_endproc
	.fill	\label + (32 * 4) - .
	.endm



	.macro no_ret _func:req, skip_nop=0
	bl	\_func
	.endm



	
#define SCRATCH_REG_SIZE #(22 * 8)
// save takes 13 instruction
	.macro save_caller_regs el_n
	sub	sp, sp, SCRATCH_REG_SIZE
	stp	x0, x1, [sp]
	stp	x2, x3, [sp, #0x10]
	stp	x4, x5, [sp, #0x20]
	stp	x6, x7, [sp, #0x30]
	stp	x8, x9, [sp, #0x40]
	stp	x10, x11, [sp, #0x50]
	stp	x12, x13, [sp, #0x60]
	stp	x14, x15, [sp, #0x70]
	stp	x16, x17, [sp, #0x80]
	stp	x18, x30, [sp, #0x90]
	mrs x30, elr_\el_n
	stp x29, x30, [sp, #0xa0] // x30 is elr
	.endm

	.macro restore_caller_regs el_n
	
	ldp	x0, x1, [sp]
	ldp	x2, x3, [sp, #0x10]
	ldp	x4, x5, [sp, #0x20]
	ldp	x6, x7, [sp, #0x30]
	ldp	x8, x9, [sp, #0x40]
	ldp	x10, x11, [sp, #0x50]
	ldp	x12, x13, [sp, #0x60]
	ldp	x14, x15, [sp, #0x70]
	ldp	x16, x17, [sp, #0x80]
	ldp	x29, x30, [sp, #0xa0] // x30 is elr
	msr elr_\el_n, x30		  // restore elr	
	ldp	x18, x30, [sp, #0x90] // is real x30
	add	sp, sp, SCRATCH_REG_SIZE
	.endm

