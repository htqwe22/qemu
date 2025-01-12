/**********************************************************************************
 * FILE : stack_frame.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-06 , At 13:01:27
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "stack_frame.h"
#include <stdint.h>
#include <arch_helpers.h>
#include <log.h>

const struct stack_frame *get_prev_frame(const struct stack_frame *fp)
{
    return  fp->fp;
}


void debug_callstack(void *fp)
{
    const struct stack_frame *frame = (const struct stack_frame *)fp;
    do {
        kv_debug_raw(LOG_LVL_INFO, "fp: %p\n", frame->fp);
        kv_debug_raw(LOG_LVL_INFO, "call from: %#lx\n", (uint64_t)frame->lr -4);
        frame = frame->fp;
    } while (frame->fp != (void *)0);
    kv_debug_raw(LOG_LVL_INFO, "fp: %p\n", frame->fp);
    kv_debug_raw(LOG_LVL_INFO, "call from: %#lx\n", (uint64_t)frame->lr -4);
    for (;;) wfi();
}


