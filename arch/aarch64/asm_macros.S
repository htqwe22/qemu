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



/**
 * 影响的寄存器有 sp, X29, X30，elr, spsr, x0 - x18 (这里我们只保存到X15)
 * SP:在这里不用保存，因为整个中断过程SP都能恢复刚刚进入中断的值（不管是否发生SP的切换）
 *    在未发生SP切换的时候，SP指向栈顶，这样就不会有踩踏。
 * X29也不用保存，是因为在汇编中没有保存，但我们也没有修改，但当中断进入正常C函数时会有保存和恢复
 * X30：可能需要保存， 发生中断的时候x30已经保存，但如果在执行bl语句的时候产生了中断，这个时候可能有问题
 * elr, spsr，这个在发生异常的时候，硬件也会直接修改，
 * 综上：所以需要保存的寄存器是硬件直接修改的，然后被调用者可以直接使用的（调用者必须保存的寄存器）
 * 由于32条指令的限制，x16-x18没有保存
 * 使用以下的中断过程，会使用到28条指令（包含ERET），还有4条指令的自由发挥空间
 */
#define SCRATCH_REG_SIZE (22 * 8)
	.macro save_caller_regs el_n
	stp x0, x1, [sp, #-SCRATCH_REG_SIZE]! // now sp move
	stp	x2, x3, [sp, #0x10]
	stp	x4, x5, [sp, #0x20]
	stp	x6, x7, [sp, #0x30]
	stp	x8, x9, [sp, #0x40]
	stp	x10, x11, [sp, #0x50]
	stp	x12, x13, [sp, #0x60]
	stp	x14, x15, [sp, #0x70]
	stp	x16, x17, [sp, #0x80]
	stp	x18, x30, [sp, #0x90]
	mrs x14, elr_\el_n		//save elr, spsr
	mrs x15, spsr_\el_n
	stp x14, x15,   [sp, #0xa0]
	.endm

	.macro restore_caller_regs el_n
	stp x14, x15, [sp, #0xa0]
	msr elr_\el_n,  x14		//restore elr, spsr
	msr spsr_\el_n, x15
	ldp	x0, x1, [sp]
	ldp	x2, x3, [sp, #0x10]
	ldp	x4, x5, [sp, #0x20]
	ldp	x6, x7, [sp, #0x30]
	ldp	x8, x9, [sp, #0x40]
	ldp	x10, x11, [sp, #0x50]
	ldp	x12, x13, [sp, #0x60]
	ldp	x14, x15, [sp, #0x70]
	ldp	x16, x17, [sp, #0x80]
	ldp	x18, x30, [sp, #0x90]
	add	sp, sp, SCRATCH_REG_SIZE
	.endm

