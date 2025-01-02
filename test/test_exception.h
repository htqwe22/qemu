/**********************************************************************************
 * FILE : test_exception.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-02 , At 21:39:52
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_TEST_EXCEPTION_H
#define KV_TEST_EXCEPTION_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif



struct exception_entry
{
    uint32_t sync_inst[32]; // contains 32 instructions.(128 bytes)
    uint32_t irq_inst[32]; 
    uint32_t fiq_inst[32]; 
    uint32_t serr_inst[32]; 
};

void test_exception(void);









#ifdef __cplusplus
}
#endif

#endif //test_exception.h
