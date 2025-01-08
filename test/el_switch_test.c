/**********************************************************************************
 * FILE : el_switch_test.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-06 , At 19:37:11
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "el_switch_test.h"
#include <arch_helpers.h>
#include <log.h>



void el1_entry(void)
{
    LOG_DEBUG("cur_el is %d\n", get_current_el());
}




