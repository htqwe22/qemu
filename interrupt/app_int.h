/**********************************************************************************
 * FILE : app_int.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 15:26:34
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_APP_INT_H
#define KV_APP_INT_H
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



// this is the very first interrupt initial handler
// make sure invoke it first before any other interrupt init
// Note: call only once ...
void gic_global_init(int fiq_in_el3, int irq_in_el3, int external_abort_in_el3);

void gic_current_pe_init(void);

/**
 * trigger & target_affi is only valid for SPI
*/
int gic_configure_interrupt(int INTID, uint8_t priority, int_group_t group, \
irq_trigger_t trigger, uint32_t target_affi, irq_handler_t handler, void *priv_arg);


int gic_set_interrupt_pending(int INTID, uint32_t target_affi);




#ifdef __cplusplus
}
#endif

#endif //app_int.h
