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
 //   pl011_init(0);
 //   asm_test(arr);
    LOG_ERROR("hello world\n");
    LOG_WARN("kevin he\n");

    return 0;
}
