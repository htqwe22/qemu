/**********************************************************************************
 * FILE : drv_uart.h
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-26 , At 10:52:25
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_DRV_UART_H
#define KV_DRV_UART_H
#include "io.h"
#ifdef __cplusplus
extern "C" {
#endif

void uart_init(uint32_t baudrate);

void uart_send(char c);

char uart_recv(void);






#ifdef __cplusplus
}
#endif

#endif //drv_uart.h
