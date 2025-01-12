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

#ifdef __cplusplus
extern "C" {
#endif

#define FP_EN   0

typedef struct 
{
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
    uint64_t elr;
    uint64_t spsr;
    uint64_t general_regs[31]; //x0-x30
#if FP_EN    
    uint64_t v[32][2]; //v0-v31
    uint64_t fpcr;
    uint64_t fpsr;
#endif
    heap_ctx_t heap;
}kv_tcb_t;










#ifdef __cplusplus
}
#endif

#endif //kvos.h
