/**********************************************************************************
 * FILE : aarch64_mmu.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-01 , At 19:01:44
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_AARCH64_MMU_H
#define KV_AARCH64_MMU_H
#include <arch.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define MAIR_DEVICE_nGnRnE	0x00ull
#define MAIR_DEVICE_nGnRE	0x04ull
#define MAIR_DEVICE_GRE	    0x0Cull
// transient:瞬态缓存行在写操作后会被立即写回主内存并标记为无效
/* Normal MEMory, Outer Write-Through non-transient, Inner Non-cacheable */
#define MAIR_MEM_NC		    0x44ull
/* Normal MEMory, Outer Write-Back non-transient, Inner Write-Back non-transient */
#define MAIR_MEM_NORMAL	    0xFFull
/* Normal MEMory, Outer Write-Through non-transient */
#define MAIR_MEM_WT		    0xBBull

#define MAIR_DEVICE_nGRE	0x08ull
/* Normal MEMory, Outer Write-Back non-transient */
#define MAIR_MEM_WB	        0x77ull

#define MAIR_DEVICE_nGnRnE_IDX	0
#define MAIR_DEVICE_nGnRE_IDX	1
#define MAIR_DEVICE_nGRE_IDX	2
#define MAIR_DEVICE_GRE_IDX	    3
#define MAIR_MEM_NORMAL_IDX	    4
#define MAIR_MEM_NC_IDX		    5
#define MAIR_MEM_WT_IDX		    6
#define MAIR_MEM_WB_IDX         7

#define MAIR_ATTR_SET(attr, index)	((attr) << ((index) << 3))


typedef union 
{
    uint64_t value;
    struct{
        uint64_t type       : 2;
        uint64_t rsvd       : 10;
        uint64_t pud_base   : 36;
    }pgd;
    struct{
        uint64_t type       : 2;
        uint64_t rsvd       : 10;
        uint64_t pmd_base   : 36;
    }pud;
    struct{
        uint64_t type       : 2;
        uint64_t rsvd       : 10;
        uint64_t pte_base   : 36;
    }pmd;
    struct{
        uint64_t type       : 2;
        uint64_t attr_lo   : 10; 
        uint64_t pa_base   : 36;
        uint64_t rsvd       : 3;
        uint64_t attr_hi    : 13;   
    }pte;
    struct{
        uint64_t type       : 2;
        uint64_t attr_lo   : 10;
        uint64_t rsvd       : 9;
        uint64_t pa_base   : 27;
        uint64_t rsvd2       : 4;
        uint64_t attr_hi    : 12; 
    }b2m;
}table_4k_entry_t;

struct page_table_4k{
    table_4k_entry_t entries[512];
};

#define ATTR_nG         (1<<11)  //非全局的，可以有ASID功能
#define ATTR_AF         (1<<10)

#define ATTR_SH_NONE    (0 << 8)
#define ATTR_SH_INNER   (3 << 8)
#define ATTR_SH_OUTER   (2 << 8)

#define ATTR_AP_RONLY   (1 << 7)
#define ATTR_AP_USER    (1 << 6)
#define ATTR_NS         (1 << 5)

#define ATTR_MAIR(idx)  (idx << 2)


#define ATTR_PXN        (1ul << 53)  //特权不可执行（再看WXN位）
#define ATTR_XN         (1ul << 54)  //任何情况都不可执行（特权下要看PXN位）

#define ATTR_COMMON     (ATTR_AF | ATTR_SH_INNER/* | ATTR_nG */)


#define ATTR_DEV_NORMAL (ATTR_COMMON | ATTR_MAIR(MAIR_DEVICE_nGnRnE_IDX))
#define ATTR_DEV_NE     (ATTR_COMMON | ATTR_MAIR(MAIR_DEVICE_nGnRE_IDX))

#define ATTR_MEM_NORMAL (ATTR_COMMON | ATTR_PXN | ATTR_XN | ATTR_MAIR(MAIR_MEM_NORMAL_IDX))
#define ATTR_MEM_NC     (ATTR_COMMON | ATTR_PXN | ATTR_XN | ATTR_MAIR(MAIR_MEM_NC_IDX))
#define ATTR_MEM_WT     (ATTR_COMMON | ATTR_PXN | ATTR_XN | ATTR_MAIR(MAIR_MEM_WT_IDX))
#define ATTR_MEM_RO_EXE (ATTR_COMMON | ATTR_AP_RONLY | ATTR_MAIR(MAIR_MEM_NORMAL_IDX))
#define ATTR_MEM_RO     (ATTR_COMMON | ATTR_AP_RONLY | ATTR_PXN | ATTR_XN | ATTR_MAIR(MAIR_MEM_NORMAL_IDX))

#define ATTR_SET_NS(attr)   ((attr) | ATTR_NS)


void init_mmu_elx(int n_el);

void enable_mmu_elx(int n_el);

#ifdef __cplusplus
}
#endif

#endif //aarch64_mmu.h
