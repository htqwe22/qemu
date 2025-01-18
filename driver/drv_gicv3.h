/**********************************************************************************
 * FILE : drv_gicv3.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-17 , At 11:10:11
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_DRV_GICV3_H
#define KV_DRV_GICV3_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    TRIGGER_LEVEL = 0,
    TRIGGER_EDGE  = 2,
}irq_trigger_t;


typedef enum
{
    GROUP0 = 0,
    GROUP1_NS = 1,
    GROUP1_S = 2,
}int_group_t;

typedef void (*irq_handler_t)(void *priv_arg);












#ifdef __cplusplus
}
#endif

#endif //drv_gicv3.h
