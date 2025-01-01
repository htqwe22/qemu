/**********************************************************************************
 * FILE : main.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-19 , At 21:01:28
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "main.h"
#include "log.h"
#include <drv_pl011.h>
#include <aarch64_common.h>

extern void asm_test(uint64_t arr[]);
extern void mem_map_init(void);
int main(int argc, char **argv)
{
    pl011_init(0);
    //init_ddr();
    //relocate();
    //init_mmu();
    mem_map_init();


    // uint64_t id_aa64pfr0 = read_id_aa64pfr0_el1();
    // LOG_ERROR("id_aa64pfr0:%016x\n", id_aa64pfr0);
    
    LOG_WARN("kevin he, cur_el %d\n", get_current_el());
    // LOG_INFO("RVBAR_EL1: %016lx\n", read_rvbar_el1());
    // LOG_INFO("RVBAR_EL2: %016lx\n", read_rvbar_el2());
    LOG_INFO("RVBAR_EL3: %016lx\n", read_rvbar_el3());
    return 0;
}
