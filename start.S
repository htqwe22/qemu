
#include <arch.h>
#include <plat_def.h>
#include <asm_macros.S>
// .section ".text.boot"
/*
 * 使用 cfi_startproc 可以为汇编增加调试信息
 */
.global _start
.global memset_64
.global memcpy_64
.global asm_test
.global proc_hang

func _start
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
endfunc _start
       
func proc_hang
    wfe
    b proc_hang
endfunc proc_hang

func master
    msr	daifset, #15 // D,A,I,F all disabled. 可以接受立即数操作
    bl init_sctlr_el3
    
    // clean bss ...
    ldr x0, =bss_begin
    mov x1, xzr
    ldr x2, =bss_end
    sub x2, x2, x0
    bl memset_64

//    mrs x3, sctlr_el1
//    bic x3, x3, #8
//    msr sctlr_el1, x3

    // disable mmu for el3
    bl disable_mmu_el3

    // set SP before go to C functions.
    ldr x7, =STACK_BASE
    mov sp, x7

    sub x7, x7, STACK_SIZE
    msr sp_el2, x7

    sub x7, x7, STACK_SIZE
    msr sp_el1, x7

    sub x7, x7, STACK_SIZE
    msr sp_el0, x7

    bl init_scr_el3
    msr SPSel, 1  // support use immediate data .
    mov x29, xzr

    // copy data from LMA to VA
    ldr x0, = __data_link_start
    ldr x1, = __data_lma_start
    ldr x2, = __data_link_end
    sub x2, x2, x0
    bl memcpy_64

    bl main
    b proc_hang
    ret
endfunc master

/*
    EE = 0; 在EL3的显式数据访问，以及EL3翻译机制中第一阶段的翻译表遍历是小端序
    WXN = 0; 对于EL3翻译机制，此字段可以强制所有可写的存储区域被视为XN（永不执行）。设置为零，以便此控制对存储器访问权限没有影响。
             （对该功能的使用需要设置M位）
    SA = 0; SP的（16字节）对齐的检查不启用。
    A = 0; 除了加载/存储独占（load/store exclusive）和加载-获取（load-acquire）/存储-释放（store-release）指令外，
           加载或存储一个或多个寄存器的指令不会检查被访问的地址是否与被访问的数据元素的大小对齐。
    DSSBS = 0; 在进入EL3时会设置PSTATE.SSBS = DSSBS（这里是0）。设置为零以在进入EL3异常时禁用推测性存储绕过安全行为
*/

func init_sctlr_el3
    ldr x0, =(SCTLR_RESET_VAL & ~(SCTLR_EE_BIT | SCTLR_WXN_BIT \
        | SCTLR_SA_BIT | SCTLR_A_BIT | SCTLR_DSSBS_BIT))
    orr x0, x0,  #SCTLR_I_BIT // 使能指令cache. orr可以接受立即数操作
    msr sctlr_el3, x0
    isb
    ret
endfunc init_sctlr_el3

func memset_64
    mov x3, x0
    cmp x2, 0
    b.eq    2f
1:
    str  x1, [x0], #8
    subs x2, x2, #8
    b.ne 1b     //no overflow again ...
2:
    mov x0, x3
    ret
endfunc memset_64

func memcpy_64  
    mov x3, x0 //backup as return value
    cmp x2, 0 // length
    b.eq    2f
1:
    ldr  x4, [x1], #8
    str  x4, [x0], #8
    subs x2, x2, #8
    b.ne 1b     // 
2:
    mov x0, x3
    ret
endfunc memcpy_64


func asm_test
    ldp x1, x2, [x0, #0x10]!

    ret
endfunc asm_test

