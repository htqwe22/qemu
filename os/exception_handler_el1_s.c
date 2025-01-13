/**********************************************************************************
 * FILE : exception_el1_s.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 19:12:27
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/
#include <exception_common.h>
extern void user_irq_handler(uint32_t INITID);
void el1_s_sync_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset,1);
    debug_callstack((void *)FP);
    write_elr_el1(read_elr_el1() + 4);
//    asm volatile ("b .");
}

void el1_s_irq_handler(uint64_t offset)
{
#if 0
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset,1);
    debug_callstack((void *)FP);
#else
    // lower 24 bits for INTID
    uint32_t INTID = (uint32_t)read_icc_iar1_el1() & 0xffffff;
    user_irq_handler(INTID);
    write_icc_eoir1_el1(INTID); // end of interrupt
    //when EOImode = 1; need to call write_icc_idr1_el1(INTID); // interrupt done
#endif
}


void el1_s_fiq_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset,1);
    debug_callstack((void *)FP);
}


void el1_s_serror_handler(uint64_t offset)
{
    uint64_t FP;
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    excep_info(offset,1);
    debug_callstack((void *)FP);
}
