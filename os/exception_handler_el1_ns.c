/**********************************************************************************
 * FILE : exception_el1_ns.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 19:12:31
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/
#include <exception_common.h>


void el1_ns_sync_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 1);
    debug_callstack((void *)FP);
    write_elr_el1(read_elr_el1() + 4);
//    asm volatile ("b .");
}

void el1_ns_irq_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 1);
    debug_callstack((void *)FP);
}


void el1_ns_fiq_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 1);
    debug_callstack((void *)FP);
}


void el1_ns_serror_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset, 1);
    debug_callstack((void *)FP);
}
