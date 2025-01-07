/**********************************************************************************
 * FILE : el_switch_test.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-06 , At 19:37:11
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_EL_SWITCH_TEST_H
#define KV_EL_SWITCH_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

void el2_entry(void);

void switch_to_el2(void *enter_point, void *vector_table);









#ifdef __cplusplus
}
#endif

#endif //el_switch_test.h
