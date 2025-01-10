/**********************************************************************************
 * FILE : mem_map.h
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-26 , At 10:59:00
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_MEM_MAP_H
#define KV_MEM_MAP_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define PERIPH_BASE 0xFE000000


// GPIO address
#define GPFSEL1                 (PERIPH_BASE+0x00200004)
#define GPSET0                  (PERIPH_BASE+0x0020001C)
#define GPCLR0                  (PERIPH_BASE+0x00200028)
#define GPPUD                   (PERIPH_BASE+0x00200094)
#define GPPUDCLK0               (PERIPH_BASE+0x00200098)
#define GPIO_PUP_PDN_CNTRL_REG0 (PERIPH_BASE+0x002000E4)


// UART_BASE
#define UART_BASE       (PERIPH_BASE+0x00201000)

#define UART_DATA       (UART_BASE)
#define UART_FR         (UART_BASE+0x18)
#define UART_IBRD       (UART_BASE+0x24)
#define UART_FBRD       (UART_BASE+0x28)
#define UART_LCRH       (UART_BASE+0x2C)
#define UART_CR         (UART_BASE+0x30)
#define UART_IMSC       (UART_BASE+0x38)



void create_4k_mapping(int el, uint64_t pa, uint64_t va, uint64_t size, uint64_t attrs);

void create_2m_mapping(int el, uint64_t pa, uint64_t va, uint64_t size, uint64_t attrs);

#ifdef __cplusplus
}
#endif

#endif //mem_map.h
