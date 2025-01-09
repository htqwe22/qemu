/**********************************************************************************
 * FILE : plat_common.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-08 , At 20:28:20
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_PLAT_COMMON_H
#define KV_PLAT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LINKER__
#define K_(x)   (x)*(1K)
#define M_(x)   (x)*(1M)
#define G_(x)   (x)*(1024M)
#else
#define K_(x)   (1024UL * (x))
#define M_(x)   (0x100000UL *(x))
#define G_(x)   (0x40000000UL *(x))
#endif







#ifdef __cplusplus
}
#endif

#endif //plat_common.h
