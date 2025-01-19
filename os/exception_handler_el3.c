/**********************************************************************************
 * FILE : exception_el3.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 19:12:23
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/
#include <exception_common.h>
#include <log.h>
#include <arch_helpers.h>

void el3_sync_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 3);
    debug_callstack((void *)FP);
    write_elr_el3(read_elr_el3() + 4);
//    asm volatile ("b .");
}


void el3_irq_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 3);
    debug_callstack((void *)FP);
}


void el3_fiq_handler(uint64_t offset)
{
    uint32_t INTID = (uint32_t)read_icc_iar0_el1() & 0xffffff;
    LOG_DEBUG("Get FIQ id %d\n", INTID);
    if (INTID == 1021) {

    }else if (INTID == 1020) {
        
    }
    write_icc_eoir0_el1(INTID); // end of interrupt
//    excep_info(offset, 3);
}


void el3_serror_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 3);
    debug_callstack((void *)FP);
}

