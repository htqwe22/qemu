/**********************************************************************************
 * FILE : kvos.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-11 , At 14:55:03
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "kvos.h"
#include <string.h>

#define FP_EN   0
typedef enum thread_state
{
    THREAD_STATE_INIT = 0,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED = 0x80,
    THREAD_STATE_DEAD,
}thread_state_e;



typedef struct 
{
    uint64_t *cur_sp;  //0x0
    uint32_t *pc;      //0x8
    uint64_t spsr;     //0x10
    uint64_t srvd;      //0x18 
    uint64_t general_regs[31]; //x20 size F8
    const char *name;   //0x118
    struct list_head member; //0x120
    uint64_t *stack_pointer; // 0x130
    uint32_t stack_size;    //0x138
    uint32_t priority;  //0x13c
    uint32_t state;     //0x140
    uint32_t rsvd;      
                        //0x150
#if FP_EN    
    uint64_t v[32][2]; //v0-v31
    uint64_t fpcr;
    uint64_t fpsr;
#endif
//    heap_ctx_t heap;
}kv_tcb_t;

const kv_tcb_t * g_current_tcb;

static LIST_HEAD(active_list);



kv_tid_t kv_thread_create(const char *name, uint32_t stack_size, uint64_t *stack_pointer, uint32_t priority, void (*entry)(void *), void *arg)
{
    kv_tcb_t *tcb = (kv_tcb_t *)kv_malloc(sizeof(kv_tcb_t));
    if (tcb) {
        memset(tcb, 0, sizeof(kv_tcb_t));
        tcb->stack_pointer = stack_pointer;
        if (tcb->stack_pointer == NULL) {
            tcb->stack_pointer = (uint64_t *)kv_malloc(stack_size);
            if (tcb->stack_pointer == NULL) {
                kv_free(tcb);
                return NULL;
            }
        }
        tcb->name = name;
        tcb->stack_size = stack_size;
        tcb->priority = priority;
        tcb->pc = (uint32_t *)entry;
        tcb->cur_sp = tcb->stack_pointer + (stack_size >> 3); // set as the top of the stack
        tcb->general_regs[30] = (uint64_t)tcb->pc;
    }
    list_add_tail(&tcb->member, &active_list);
    return tcb;
}


static void idle_thread(void *arg)
{
    uint32_t num;
    while (1)
    {
        num++;
    }
    // never exit ...
}

void task_start_schedule(void)
{
    extern void start_first_task(kv_tcb_t *tcb);
    asm volatile("msr daifset, #3\n");
    (void)kv_thread_create("idle", 2048, NULL, 8, idle_thread, NULL);
    kv_tcb_t *tcb = list_first_entry(&active_list, kv_tcb_t, member);
    start_first_task(tcb);
    // never return...
}

kv_tcb_t *fetch_next_run_task(void)
{
    while (1) {
        kv_tcb_t *tcb = list_first_entry(&g_current_tcb->member, kv_tcb_t, member);
        if (&tcb->member != &active_list) {
            // check the priority...
            return tcb;
        }
    }
}