
.section ".text"

.globl	disable_mmu_el1
.globl	disable_mmu_el2
.globl	disable_mmu_el3


disable_mmu_el1:
	mov	x1, #1      // #5 C & M

	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret




disable_mmu_el2:
	mov	x1, #1      // #5 C & M

	mrs	x0, sctlr_el2
	bic	x0, x0, x1
	msr	sctlr_el2, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret




disable_mmu_el3:
	mov	x1, #1      // #5 C & M

	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret