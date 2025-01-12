#include <arch.h>
#include <asm_macros.S>
// .section ".text"

.globl	disable_mmu_el1
.globl	disable_mmu_el2
.globl	disable_mmu_el3
.global switch_to_el1
.global test_print


  .global getAffinity
func getAffinity
  MRS      x0, MPIDR_EL1
  UBFX     x1, x0, #32, #8      // rd, rn, #p, #n ==> x1 = x0[39:31]
  BFI      w0, w1, #24, #8      // rd, rn, #p, #n ==> w0[31:24] = w1[7:0]
  RET
endfunc



func disable_mmu_el1
	mov	x1, #1      // #5 C & M

	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret
endfunc



func disable_mmu_el2
	mov	x1, #1      // #5 C & M

	mrs	x0, sctlr_el2
	bic	x0, x0, x1
	msr	sctlr_el2, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret
endfunc



func disable_mmu_el3
	mov	x1, #1      // #5 C & M

	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret
endfunc

/* 可以参考ATF中 bl1_prepare_next_image的实现
 * ELt等级的切换需要注意以下几点：
 * 1. 注意切换时要mask掉DAIF
 * 2. 切换后的ELt等级及相应的栈的栈指针需要设置 （SPSR_EL3.M 这个域也同时设置了EL2使用哪个栈）
 * 3. 切换后的PSTATE执行程序状态需要设置（SPSR_EL3）//SSBS = 0，设置M[4] = 0， M[3:0] = 0101 EL1h
 * 4. 切换后的执行入口点需要设置（ELR_EL3）
 * 5. 切换后的执行状态需要设置（AArch64 SCR_EL3.RW = 1）
 * 6. 切换后的ELt安全性需要设置 (安全性, SCR_EL3.{NSE, NS} = 0b00, SCR_EL3.EEL2 = 1)
 * 7. 切换后的数据大小端模式需要设置（SCTLR_EL2.EE）
 * 8. 等级的异常向量表需要设置 （先不设置，用的时候设置，也可以直接用EL3的先过度一下）
 * 9. CPTR寄存器
 * 10. 注意barrier的设置
*/
func switch_to_el1 // el2 not supported, use el1
	msr daifset, #15 // diable exceptions
	msr elr_el3, x0  // pc
	msr vbar_el1, x1 // vector table

	mrs x4, scr_el3
	// security setting ...
	// set {NSE, NS} = 01 NS ...
	ldr x5, =(SCR_NSE_BIT | SCR_NS_BIT)
	bic x4, x4, x5 
	//orr x4, x4, SCR_NS_BIT

	// set aarch64 in EL1
	orr x4, x4, SCR_RW_BIT
	
//	mov x4, #0x630
	msr scr_el3, x4 //scr_el3 ...

//	mrs x4, hcr_el2
//	msr hcr_el2, xzr

	// configure sctlr_el1
	//ldr x4, =(SCTLR_EL1_RES1) 
	//EE = 0, EOE = 0
	msr sctlr_el1, xzr

	// configure spsr_el3
	// E = 0, to use littleendian
	ldr x4, =(0xf<<6 | 0x5) // diable exception at el1 & EL12h mode, 
	msr spsr_el3, x4 //spsr_el3 ...

	ldr x4, =(3 << 20)
	msr cpacr_el1, x4 // FP/SIMD registers

	eret
	dsb	sy
	isb
endfunc