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