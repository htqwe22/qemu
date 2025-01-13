/**********************************************************************************
 * FILE : drv_sys_timer.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-13 , At 16:53:25
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_DRV_SYS_TIMER_H
#define KV_DRV_SYS_TIMER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

int sys_timer_init(uint32_t interval_us);

void sys_timer_stop(void);

void sys_timer_interrupt_handler(void *arg);






#ifdef __cplusplus
}
#endif

#endif //drv_sys_timer.h
