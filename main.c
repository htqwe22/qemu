/**********************************************************************************
 * FILE : main.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-19 , At 21:01:28
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "main.h"
#include "log.h"
#include <drv_pl011.h>
#include <aarch64_common.h>

extern void asm_test(uint64_t arr[]);

int main(int argc, char **argv)
{
    pl011_init(0);
 //   asm_test(arr);
    uint64_t id_aa64pfr0 = read_id_aa64pfr0_el1();
    // GIC 3 AND 4 implemented.
    // EL0~EL3 implemented
    LOG_ERROR("id_aa64pfr0:%016x\n", id_aa64pfr0);
    LOG_WARN("kevin he\n");

    return 0;
}
