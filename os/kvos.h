/**********************************************************************************
 * FILE : kvos.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-11 , At 14:55:03
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_KVOS_H
#define KV_KVOS_H
#include <stdint.h>
#include <plat_def.h>
#include <heap.h>
#include <list.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef void * kv_tid_t;

kv_tid_t kv_thread_create(const char *name, uint32_t stack_size, uint64_t *stack_pointer, uint32_t priority, void (*entry)(void *), void *arg);

void task_start_schedule(void);

void task_context_switch(void);

#ifdef __cplusplus
}
#endif

#endif //kvos.h
