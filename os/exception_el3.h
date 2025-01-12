/**********************************************************************************
 * FILE : exception_el3.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 19:12:23
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_EXCEPTION_EL3_H
#define KV_EXCEPTION_EL3_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



void el3_exception_sync_handler(uint64_t offset);


void el3_irq_handler(uint64_t offset);



void el3_fiq_handler(uint64_t offset);



void el3_serror_handler(uint64_t offset);








#ifdef __cplusplus
}
#endif

#endif //exception_el3.h
