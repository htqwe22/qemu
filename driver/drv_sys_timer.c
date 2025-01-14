/**********************************************************************************
 * FILE : drv_sys_timer.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-13 , At 16:53:25
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "drv_sys_timer.h"
#include <arch_helpers.h>
#include <log.h>

#define PLATFORM_CORE_COUNT     1
#define plat_my_core_pos()      0

/**
 *  CNTP EL1 physical timer
    CNTP_CTL_ELx;   control register
    CNTP_CVAL_ELx   Comparator Value Register
    CNTP_TVAL_ELx   Timer Value Register
    使用32位的TVAL可能更合适我的需求，直接让CVAL的值等于将来的系统时间
    CTL.ENABLE       enable timer
    CTL.IMASK        interrupt mask = 0, 使能中断，这个是PPI中断，只能当前的核心响应
    CTL.ISTATUS      reports whether the timer is firing ...
    中断ID： CNTP 30
    该Timer被设置成电平触发，结束这个信息需要以下几个条件之一
    1. IMASK = 1
    2. ENABLE = 0
    3. TVAL or CVAL 被定入
*/

static uint64_t tval;
static void (*timer_callback)(void *);
static void *timer_arg;
int sys_timer_init(uint32_t interval_us, void (*callback)(void *), void *arg)
{
    uint64_t g_cntp_freq;
    uint64_t ctl = 1;
    // bi0  : enable
    // bit1 : imask
    // bit2 : state
    g_cntp_freq = read_cntfrq_el0();
    LOG_DEBUG("cnt freq: %luK\n", g_cntp_freq/1000);
    tval = interval_us * g_cntp_freq / 1000000; // 1000000us = 1s
    write_cntp_tval_el0(tval);
    timer_callback = callback;
    timer_arg = arg;
    // enable ...
    write_cntp_ctl_el0(ctl);
    return 0;
}

void sys_timer_stop(void)
{
    write_cntp_ctl_el0(0);
}

void sys_timer_restore(void)
{
    write_cntp_tval_el0(tval);
}

void sys_timer_interrupt_handler(void *arg)
{
    sys_timer_stop();
//    LOG_DEBUG("sys timer interrupt\n");
    if (timer_callback) {
        timer_callback(timer_arg);
    }
    sys_timer_restore();
    isb();
    write_cntp_ctl_el0(1);

//    asm volatile ("svc #0\n");
}

#if 0

/*******************************************************************************
 * Data structure to keep track of per-cpu secure generic timer context across
 * power management operations.
 ******************************************************************************/
typedef struct timer_context {
	uint64_t cval;
	uint32_t ctl;
} timer_context_t;

static timer_context_t pcpu_timer_context[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * This function initializes the generic timer to fire every 0.5 second
 ******************************************************************************/
void tsp_generic_timer_start(void)
{
	uint64_t cval;
	uint32_t ctl = 0;

	/* The timer will fire every 0.5 second */
	cval = read_cntpct_el0() + (read_cntfrq_el0() >> 1);
	write_cntps_cval_el1(cval);

	/* Enable the secure physical timer */
	set_cntp_ctl_enable(ctl);
	write_cntps_ctl_el1(ctl);
}

/*******************************************************************************
 * This function deasserts the timer interrupt and sets it up again
 ******************************************************************************/
void tsp_generic_timer_handler(void)
{
	/* Ensure that the timer did assert the interrupt */
//	assert(get_cntp_ctl_istatus(read_cntps_ctl_el1()));

	/*
	 * Disable the timer and reprogram it. The barriers ensure that there is
	 * no reordering of instructions around the reprogramming code.
	 */
	isb();
	write_cntps_ctl_el1(0);
	tsp_generic_timer_start();
	isb();
}

/*******************************************************************************
 * This function deasserts the timer interrupt prior to cpu power down
 ******************************************************************************/
void tsp_generic_timer_stop(void)
{
	/* Disable the timer */
	write_cntps_ctl_el1(0);
}

/*******************************************************************************
 * This function saves the timer context prior to cpu suspension
 ******************************************************************************/
void tsp_generic_timer_save(void)
{
	uint32_t linear_id = plat_my_core_pos();

	pcpu_timer_context[linear_id].cval = read_cntps_cval_el1();
	pcpu_timer_context[linear_id].ctl = read_cntps_ctl_el1();
//	flush_dcache_range((uint64_t) &pcpu_timer_context[linear_id],
//			   sizeof(pcpu_timer_context[linear_id]));
}

/*******************************************************************************
 * This function restores the timer context post cpu resumption
 ******************************************************************************/
void tsp_generic_timer_restore(void)
{
	uint32_t linear_id = plat_my_core_pos();

	write_cntps_cval_el1(pcpu_timer_context[linear_id].cval);
	write_cntps_ctl_el1(pcpu_timer_context[linear_id].ctl);
}
#endif
