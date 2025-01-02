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

#ifdef __cplusplus
extern "C" {
#endif
#define MAIR_DEVICE_nGnRnE	0x00ull
#define MAIR_DEVICE_nGnRE	0x04ull
#define MAIR_DEVICE_GRE	    0x0Cull
/* Normal MEMory, Outer Write-Through non-transient, Inner Non-cacheable */
#define ATTR_MEM_NC		    0x44ull
/* Normal MEMory, Outer Write-Back non-transient, Inner Write-Back non-transient */
#define MAIR_MEM_NORMAL	    0xFFull
/* Normal MEMory, Outer Write-Through non-transient */
#define ATTR_MEM_WT		    0xBBull

#define MAIR_DEVICE_nGRE	0x08ull
/* Normal MEMory, Outer Write-Back non-transient */
#define MAIR_MEM_WB	        0x77ull

#define MAIR_DEVICE_nGnRnE_IDX	0
#define MAIR_DEVICE_nGnRE_IDX	1
#define MAIR_DEVICE_nGRE_IDX	2
#define MAIR_DEVICE_GRE_IDX	3
#define MAIR_MEM_NORMAL_IDX	4
#define ATTR_MEM_NC_IDX		5
#define ATTR_MEM_WT_IDX		6

#define MAIR_ATTR_SET(attr, index)	((attr) << ((index) << 3))


void init_mmu_el3(void);



#ifdef __cplusplus
}
#endif

#endif //aarch64_mmu.h
