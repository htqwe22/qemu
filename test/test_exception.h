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

/*
 * M[3:0] 中 M[3:2] 返回的异常等级
 * M[1] 保留位 
 * M[0] 是SPSel的值，0表示使用SP_EL0， 1表示使用SP_ELx
*/
union reg_spsr
{
    uint32_t data;
    struct {
        uint32_t M : 4; // Execption mode,  M[3:2] el, M[1] rsvd. M[0]=spsel
        uint32_t EXE_M : 1; // Excute mode
        uint32_t rsvd0 : 1;
        uint32_t F : 1;
        uint32_t I : 1;
        uint32_t A : 1;
        uint32_t D : 1;
        uint32_t rsvd1 : 10;
        uint32_t IL : 1; // illegal execution state
        uint32_t SS : 1; // software step
        uint32_t rsvd2 : 6;
        uint32_t V : 1;
        uint32_t C : 1;
        uint32_t Z : 1;
        uint32_t N : 1;
    };
};

union reg_esr
{
    uint64_t data;
    struct {
        uint32_t ISS : 25;
        uint32_t IL : 1; // Excute mode
        uint32_t EC : 6;
        uint32_t ISS2 : 5;
    };
};


void test_exception(void);

void set_exception_table_el3(void);







#ifdef __cplusplus
}
#endif

#endif //test_exception.h
