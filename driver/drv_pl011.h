/**********************************************************************************
 * FILE : drv_pl011.h
 * Description:
 * Author: Kevin He
 * Created On: 2024-06-13 , At 22:53:57
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_DRV_PL011_H
#define KV_DRV_PL011_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif




int pl011_init(uint32_t baudrate);

int pl011_putc(char ch);

int pl011_getc(void);

void pl011_flush(void);




#ifdef __cplusplus
}
#endif

#endif //drv_pl011.h
