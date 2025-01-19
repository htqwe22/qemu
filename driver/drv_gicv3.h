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

// Note: in implementation, ITS_PAGE_SIZE may RO
#ifndef FIXED_PAGE_SIZE
#define ITS_PAGE_SIZE   (4096) //4KB
#endif

#define USED_DEV_ID_BITS    4
#define USED_EV_ID_BITS     4


int gicv3_global_init(bool irq_in_el3, bool fiq_in_el3, bool external_abort_in_el3);


int gicv3_current_cpu_interface_init(void);


int gicv3_lpi_enable(uint32_t affinity, uint32_t max_lpi_num);

void gicv3_its_enable(uint64_t its_base, bool en);

int gicv3_its_init(uint64_t its_base);


int gicv3_spi_register(uint32_t INTID, uint8_t priority, int_group_t group, irq_trigger_t trigger, uint32_t target_affi, irq_handler_t handler, void *priv_arg);

int gicv3_ppi_register(uint32_t INTID, uint8_t priority, int_group_t group, irq_trigger_t trigger, uint32_t target_affi, irq_handler_t handler, void *priv_arg);

int gicv3_sgi_register(uint32_t INTID, uint8_t priority, int_group_t group, uint32_t target_affi, irq_handler_t handler, void *priv_arg);

int gicv3_its_register(uint64_t its_base, uint32_t INTID, uint8_t priority, uint32_t target_affi,  \
    uint32_t dev_id, uint32_t ev_id, uint32_t collect_id, irq_handler_t handler, void *priv_arg);
/**
 * only for spi, ppi, sgi
 * @param affinity: only used for ppi, sgi
*/
void gicv3_set_pending(uint32_t INTID);

void gicv3_set_lpi_pending(uint64_t its_base, uint32_t target_affi, uint32_t dev_id, uint32_t event_id);

void gicv3_send_sgi(int_group_t group, uint32_t INTID, uint32_t target_affi, uint16_t list_bitmap);

#ifdef __cplusplus
}
#endif

#endif //drv_gicv3.h
