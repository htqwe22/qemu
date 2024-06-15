/**********************************************************************************
 * FILE : io.h
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-19 , At 22:20:54
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_IO_H
#define KV_IO_H
//#include "arch_helpers.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define dmb()		__asm__ __volatile__ ("" : : : "memory")

#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))

#define readl(c)	({ unsigned int  __v = __arch_getl(c); dmb(); __v; })
#define writel(v,c)	({ unsigned int  __v = v; dmb(); __arch_putl(__v,c);})

#define RO_REG  const volatile
#define RW_REG  volatile
#define WO_REG  volatile










#ifdef __cplusplus
}
#endif

#endif //io.h
