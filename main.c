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
#include <aarch64_mmu.h>
#include <test_exception.h>


extern void asm_test(uint64_t arr[]);
extern void mem_map_init(void);
extern int do_shell_loop(void);


void add(uint64_t a, uint64_t b)
{
    uint64_t a1, a2, a3, a4, a5;
    a1 = a + b;
    a2 = a *b;
    a3 = a / b;
    a4 = a & b;
    a5 = a1 + a2 + a3 + a4;
    return a5;
}

int main(int argc, char **argv)
{
    //clock init ...
    pl011_init(0);
    //init_ddr();
    //relocate();
    //init_mmu();
    //init_mmu_el3();

    add(0x1000, 0x1001);
    test_exception();
    // uint64_t id_aa64pfr0 = read_id_aa64pfr0_el1();
    // LOG_ERROR("id_aa64pfr0:%016x\n", id_aa64pfr0);
    
    LOG_WARN("kevin he, cur_el %d\n", get_current_el());
    // LOG_INFO("RVBAR_EL1: %016lx\n", read_rvbar_el1());
    // LOG_INFO("RVBAR_EL2: %016lx\n", read_rvbar_el2());
    LOG_INFO("RVBAR_EL3: %016lx\n", read_rvbar_el3());



    do_shell_loop();
    return 0;
}
