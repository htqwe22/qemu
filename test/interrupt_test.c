/**********************************************************************************
 * FILE : interrupt_test.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-19 , At 17:10:26
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "interrupt_test.h"
#include <drv_gicv3.h>
#include <arch_helpers.h>
#include <asm_func.h>
#include <log.h>
#include <drv_sys_timer.h>
#include <plat_def.h>

static void spi_interrupt_handler(void *arg)
{
    LOG_INFO("spi interrupt handler\n");
}

static void sgi_interrupt_handler(void *arg)
{
    LOG_INFO("sgi interrupt handler\n");
}


void test_lpi_interrupt_handler(void *arg)
{
    LOG_INFO("lpi interrupt\n");
}


int interrupt_init_el3(void)
{
    uint32_t affinity = getAffinity(); //read from MPIDR_EL1
    gicv3_global_init(0, 1, 0);
    gicv3_current_cpu_interface_init();
    gicv3_lpi_enable(affinity, 128);
   

    gicv3_spi_register(33, 3, GROUP1_S, TRIGGER_EDGE, affinity, spi_interrupt_handler, NULL);
    gicv3_ppi_register(30, 3, GROUP1_S, TRIGGER_LEVEL, affinity, sys_timer_interrupt_handler, NULL);
    gicv3_sgi_register(2, 5, GROUP1_S, affinity, sgi_interrupt_handler, NULL);
    return 0;
}


void its_interrupt_test(void)
{
    gicv3_set_pending(33);
//   
    gicv3_its_init(GICI_BASE);
    uint32_t affinity = getAffinity();
    gicv3_its_register(GICI_BASE, 8192, 2, affinity, 0, 0, 0, test_lpi_interrupt_handler, NULL);

 //   gicv3_send_sgi(GROUP1_S, 2, 0, 1);
    // gic_its_init(0, 1024);
 //   gicv3_set_lpi_pending(GICI_BASE, affinity, 0, 0);
}