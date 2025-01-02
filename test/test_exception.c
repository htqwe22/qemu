/**********************************************************************************
 * FILE : test_exception.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-02 , At 21:39:52
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "test_exception.h"
#include <arch_helpers.h>
#include "log.h"

//extern uint64_t el3_exceptions[];
extern struct exception_entry el3_exceptions[4];


void el3_exception_handler(uint64_t reason)
{
    LOG_DEBUG("reason %lx\n", reason); 
}


void test_exception(void)
{
    LOG_DEBUG("EL3 exception table at %p, size of table %lu\n", el3_exceptions, sizeof(el3_exceptions)); 
    uint8_t data[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    LOG_DEBUG("%02X", data[10]);
    write_vbar_el3((u_register_t)el3_exceptions);
    write_daifclr(0xf); 
    uint64_t *ptr = (uint64_t *)(data + 2);
    *ptr = 0x123456789abcdef0;
    kv_debug_data("data", data, 12);
}

