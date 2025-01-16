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
#include <el_switch_test.h>
#include <exception_common.h>
#include <arch_helpers.h>
#include <asm_func.h>
#include <app_int.h>
#include <drv_sys_timer.h>
#include <kvos.h>


extern void asm_test(uint64_t arr[]);
extern void mem_map_init(void);
extern int do_shell_loop(void);

extern int image_end;
extern int bss_begin;
static void main_task(void *arg);
static void second_task(void *arg);
static void sys_timer_callback(void *arg);

static void spi_interrupt_handler(void *arg)
{
    LOG_INFO("spi interrupt handler\n");
}
int main(int argc, char **argv)
{
    
    //clock init ...
    pl011_init(0);
    //init_ddr();
    set_exception_table_el3();
    set_exception_table_el1_s();
    //relocate();
    //init_mmu();
    //init_mmu_el3();
    

    LOG_WARN("kevin he, cur_el %d\n", get_current_el());
    LOG_INFO("MIDR_EL1: %016lx\n", read_midr_el1());
    // LOG_INFO("RVBAR_EL1: %016lx\n", read_rvbar_el1());
    // LOG_INFO("RVBAR_EL2: %016lx\n", read_rvbar_el2());
    LOG_INFO("RVBAR_EL3: %016lx\n", read_rvbar_el3());
    LOG_INFO("id_aa64pfr0:%016x\n", read_id_aa64pfr0_el1());
    LOG_INFO("SCR_EL3: %016lx, daif %x\n", read_scr_el3(), read_daif());
    LOG_INFO("MPIDR_EL1: %016lx\n", read_mpidr_el1());
    LOG_INFO("image end at %lu, bss_start at %lu\n", (uint64_t)&image_end, (uint64_t)&bss_begin);
//  LOG_DEBUG("ICC_SRE_EL3: %x, %x, %x\n", getICC_SRE_EL3(), getICC_SRE_EL2(), getICC_SRE_EL1());
//    mem_map_init();
    gic_global_init(1, 0, 0);
    gic_current_pe_init();
    gic_configure_interrupt(30, 3, GROUP1_S, TRIGGER_LEVEL, getAffinity(), sys_timer_interrupt_handler, NULL);
    gic_configure_interrupt(33, 3, GROUP1_S, TRIGGER_EDGE, getAffinity(), spi_interrupt_handler, NULL);
    
    switch_to_el1(el1_entry, 0);
    sys_timer_init(500000, sys_timer_callback, NULL);
    gic_set_interrupt_pending(33, getAffinity());

    gic_its_init(0, 1024);

   kv_thread_create("mainThread", 0x1000, NULL, 1, main_task, NULL);
   kv_thread_create("subTask", 0x1000, NULL, 1, second_task, NULL);
   task_start_schedule();
    for (;;);
    return 0;
}

static volatile uint32_t g_cnt = 0;
static void sys_timer_callback(void *arg)
{
    kv_printf("sys timer callback\n");
    g_cnt++;
}

static void main_task(void *arg)
{
    uint32_t cnt;
    for (;;) {
        cnt = g_cnt;
        kv_printf("main task run ...\n");
        while(cnt == g_cnt);
        schedule();
        kv_printf("main task run over\n");
 //       do_shell_loop();
    }
}

static void second_task(void *arg)
{
    uint32_t cnt;
    for (;;) {
        cnt = g_cnt;
        kv_printf("second_task run ...\n");
        while(cnt == g_cnt);
        schedule();
        kv_printf("second_task run over\n");
 //       do_shell_loop();
    }
}
