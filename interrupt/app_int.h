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

int gic_its_init(uint32_t rd, uint32_t lpi_id_num);

#ifdef __cplusplus
}
#endif

#endif //app_int.h
