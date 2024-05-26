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
#include <uart/drv_uart.h>

int main(int argc, char **argv)
{
    uart_init(115200);
    LOG_DEBUG("main %02x\n", 4);
    //TODO ...

    return 0;
}
