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
