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
void gic_global_init(void);

void gic_current_pe_init(void);

int gic_configure_spi(int INTID, int priority, int target_cpu, int enable);






#ifdef __cplusplus
}
#endif

#endif //app_int.h
