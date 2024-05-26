/**********************************************************************************
 * FILE : drv_uart.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-26 , At 10:52:25
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "drv_uart.h"
#include <mem_map.h>
#include <io.h>


void uart_init(uint32_t baudrate)
{
	unsigned int selector;

	/* clean gpio14 */
	selector = readl(GPFSEL1);
	selector &= ~(7<<12);
	/* set alt0 for gpio14 */
	selector |= 4<<12;
	/* clean gpio15 */
	selector &= ~(7<<15);
	/* set alt0 for gpio15 */
	selector |= 4<<15;
	writel(selector, GPFSEL1);


	/*set gpio14/15 pull down state*/
	selector = readl(GPIO_PUP_PDN_CNTRL_REG0);
	selector |= (0x2 << 30) | (0x2 << 28);
	writel(selector, GPIO_PUP_PDN_CNTRL_REG0);	

	/* disable UART until configuration is done */
	writel(0, UART_CR);

	/*
	 * baud divisor = UARTCLK / (16 * baud_rate)
	= 48 * 10^6 / (16 * 115200) = 26.0416666667
	integer part = 26
	fractional part = (int) ((0.0416666667 * 64) + 0.5) = 3
	generated baud rate divisor = 26 + (3 / 64) = 26.046875
	generated baud rate = (48 * 10^6) / (16 * 26.046875) = 115177
	error = |(115177 - 115200) / 115200 * 100| = 0.02%
	*/

	/* baud rate divisor, integer part */
#ifdef REAL_BOARD
	writel(26, UART_IBRD);
	/* baud rate divisor, fractional part */
	writel(3, UART_FBRD);
#else
	writel(0, UART_IBRD);
	/* baud rate divisor, fractional part */
	writel(1, UART_FBRD);
#endif
	/* enable FIFOs and 8 bits frames */
	writel((1<<4) | (3<<5), UART_LCRH);

	/* mask interupts */
	writel(0, UART_IMSC);
	/* enable UART, receive and transmit */
	writel(1 | (1<<8) | (1<<9), UART_CR);
}


void uart_send(char c)
{
	/* wait for transmit FIFO to have an available slot*/
	while (readl(UART_FR) & (1<<5))
		;

	writel(c, UART_DATA);
}


char uart_recv(void)
{
	/* wait for receive FIFO to have data to read */
	while (readl(UART_FR) & (1<<4));

	return (readl(UART_DATA) & 0xFF);
}