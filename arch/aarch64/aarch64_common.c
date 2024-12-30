/**********************************************************************************
 * FILE : aarch64_common.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-06-15 , At 10:13:01
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "aarch64_common.h"


void init_scr_el3(void)
{
    uint64_t val = SCR_RES1_BITS;
    val |= SCR_SIF_BIT; //禁止从非安全memory取指令
    val |= SCR_EA_BIT;  //将外部的Abort和SError路由到EL3处理。不管当时是在哪个EL上执行
    
    u_register_t id_aa64pfr0 = read_id_aa64pfr0_el1();
    if (id_aa64pfr0 & (ID_AA64PFR0_SEL2_MASK << ID_AA64PFR0_SEL2_SHIFT));
        val |= SCR_EEL2_BIT; // 如果S-EL2存在且使能的情况下,设置成1
    write_scr_el3(val); //写入SCR_EL3
}


bool is_current_mmu_enable(void)
{
    disable_mmu_el1();
    __asm__ volatile ("mov x28, #7\n");	
    u_register_t sctlr;
    int el = get_current_el();
    if (el == 3)
        sctlr = read_sctlr_el3();
    else if (el == 2)
        sctlr = read_sctlr_el2();
    else
        sctlr = read_sctlr_el2();
    __asm__ volatile ("mov x28, #8\n");	
    return (sctlr & 1);
}
