/**********************************************************************************
 * FILE : asm_func.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 15:10:32
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_ASM_FUNC_H
#define KV_ASM_FUNC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


void disable_mmu_el1(void);
void disable_mmu_el3(void);
void disable_mpu_el2(void);
void disable_mmu_icache_el1(void);
void disable_mmu_icache_el3(void);
void disable_mpu_icache_el2(void);


extern void *memset_64(void *s, uint64_t c, size_t n);

extern void *memcpy_64(void *dest, const void *src, size_t n);

void switch_to_el1(void *enter_point, void *vector_table);

 //A3.A2.A1.A0
uint32_t getAffinity(void);



#ifdef __cplusplus
}
#endif

#endif //asm_func.h
