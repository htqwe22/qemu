/**********************************************************************************
 * FILE : drv_pl011.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-06-13 , At 22:53:57
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "drv_pl011.h"
#include "io.h"

struct pl011_reg {
    RW_REG  uint16_t DR;
            uint16_t RESERVED0;
    union {
        RW_REG  uint8_t     RSR;
        RW_REG  uint8_t     ECR;
    };
            uint8_t  RESERVED1[0x18 - 0x05];
    RO_REG   uint16_t FR;
            uint16_t RESERVED2[3];
    RW_REG  uint8_t  ILPR;
            uint8_t  RESERVED3[3];
    RW_REG  uint16_t IBRD;
            uint16_t RESERVED4;
    RW_REG  uint32_t FBRD;
    RW_REG  uint16_t LCR_H;
            uint16_t RESERVED5;
    RW_REG  uint16_t CR;
            uint16_t RESERVED6;
    RW_REG  uint16_t IFLS;
            uint16_t RESERVED7;
    RW_REG  uint16_t IMSC;
            uint16_t RESERVED8;
    RO_REG   uint16_t RIS;
            uint16_t RESERVED9;
    RO_REG   uint16_t MIS;
            uint16_t RESERVED10;
    WO_REG   uint16_t ICR;
            uint16_t RESERVED11;
    RW_REG  uint16_t DMACR;
            uint8_t  RESERVED12[0xFE0 - 0x4C];
    RO_REG   uint32_t PID0;
    RO_REG   uint32_t PID1;
    RO_REG   uint32_t PID2;
    RO_REG   uint32_t PID3;
    RO_REG   uint32_t CID0;
    RO_REG   uint32_t CID1;
    RO_REG   uint32_t CID2;
    RO_REG   uint32_t CID3;
};

#define PL011_DR_DATA (uint16_t)0x00FF
#define PL011_DR_FE   (uint16_t)0x0100
#define PL011_DR_PE   (uint16_t)0x0200
#define PL011_DR_BE   (uint16_t)0x0400
#define PL011_DR_OE   (uint16_t)0x0800

#define PL011_RSR_FE  (uint8_t)0x01
#define PL011_RSR_PE  (uint8_t)0x02
#define PL011_RSR_BE  (uint8_t)0x04
#define PL011_RSR_OE  (uint8_t)0x08
#define PL011_ECR_CLR (uint8_t)0xFF

#define PL011_FR_CTS  (uint16_t)0x0001
#define PL011_FR_DSR  (uint16_t)0x0002
#define PL011_FR_DCD  (uint16_t)0x0004
#define PL011_FR_BUSY (uint16_t)0x0008
#define PL011_FR_RXFE (uint16_t)0x0010
#define PL011_FR_TXFF (uint16_t)0x0020
#define PL011_FR_RXFF (uint16_t)0x0040
#define PL011_FR_TXFE (uint16_t)0x0080
#define PL011_FR_RI   (uint16_t)0x0100

#define PL011_LCR_H_BRK        (uint16_t)0x0001
#define PL011_LCR_H_PEN        (uint16_t)0x0002
#define PL011_LCR_H_EPS        (uint16_t)0x0004
#define PL011_LCR_H_STP2       (uint16_t)0x0008
#define PL011_LCR_H_FEN        (uint16_t)0x0010
#define PL011_LCR_H_WLEN       (uint16_t)0x0060
#define PL011_LCR_H_WLEN_5BITS (uint16_t)0x0000
#define PL011_LCR_H_WLEN_6BITS (uint16_t)0x0020
#define PL011_LCR_H_WLEN_7BITS (uint16_t)0x0040
#define PL011_LCR_H_WLEN_8BITS (uint16_t)0x0060
#define PL011_LCR_H_SPS        (uint16_t)0x0080

#define PL011_CR_UARTEN (uint16_t)0x0001
#define PL011_CR_SIREN  (uint16_t)0x0002
#define PL011_CR_SIRLP  (uint16_t)0x0004
#define PL011_CR_LBE    (uint16_t)0x0080
#define PL011_CR_TXE    (uint16_t)0x0100
#define PL011_CR_RXE    (uint16_t)0x0200
#define PL011_CR_DTR    (uint16_t)0x0400
#define PL011_CR_RTS    (uint16_t)0x0800
#define PL011_CR_OUT1   (uint16_t)0x1000
#define PL011_CR_OUT2   (uint16_t)0x2000
#define PL011_CR_RTSEN  (uint16_t)0x4000
#define PL011_CR_CTSEN  (uint16_t)0x8000

#define PL011_IFLS_TXIFLSEL (uint16_t)0x0007
#define PL011_IFLS_RXIFLSEL (uint16_t)0x0038

#define PL011_IMSC_RIMIM  (uint16_t)0x0001
#define PL011_IMSC_CTSMIM (uint16_t)0x0002
#define PL011_IMSC_DCDMIM (uint16_t)0x0004
#define PL011_IMSC_DSRMIM (uint16_t)0x0008
#define PL011_IMSC_RXIM   (uint16_t)0x0010
#define PL011_IMSC_TXIM   (uint16_t)0x0020
#define PL011_IMSC_RTIM   (uint16_t)0x0040
#define PL011_IMSC_FEIM   (uint16_t)0x0080
#define PL011_IMSC_PEIM   (uint16_t)0x0100
#define PL011_IMSC_BEIM   (uint16_t)0x0200
#define PL011_IMSC_OEIM   (uint16_t)0x0400

#define PL011_RIS_RIRMIS  (uint16_t)0x0001
#define PL011_RIS_CTSRMIS (uint16_t)0x0002
#define PL011_RIS_DCDRMIS (uint16_t)0x0004
#define PL011_RIS_DSRRMIS (uint16_t)0x0008
#define PL011_RIS_RXRIS   (uint16_t)0x0010
#define PL011_RIS_TXRIS   (uint16_t)0x0020
#define PL011_RIS_RTRIS   (uint16_t)0x0040
#define PL011_RIS_FERIS   (uint16_t)0x0080
#define PL011_RIS_PERIS   (uint16_t)0x0100
#define PL011_RIS_BERIS   (uint16_t)0x0200
#define PL011_RIS_OERIS   (uint16_t)0x0400

#define PL011_MIS_RIMMIS  (uint16_t)0x0001
#define PL011_MIS_CTSMMIS (uint16_t)0x0002
#define PL011_MIS_DCDMMIS (uint16_t)0x0004
#define PL011_MIS_DSRMMIS (uint16_t)0x0008
#define PL011_MIS_RXMIS   (uint16_t)0x0010
#define PL011_MIS_TXMIS   (uint16_t)0x0020
#define PL011_MIS_RTMIS   (uint16_t)0x0040
#define PL011_MIS_FEMIS   (uint16_t)0x0080
#define PL011_MIS_PEMIS   (uint16_t)0x0100
#define PL011_MIS_BEMIS   (uint16_t)0x0200
#define PL011_MIS_OEMIS   (uint16_t)0x0400

#define PL011_ICR_RIMIC  (uint16_t)0x0001
#define PL011_ICR_CTSMIC (uint16_t)0x0002
#define PL011_ICR_DCDMIC (uint16_t)0x0004
#define PL011_ICR_DSRMIC (uint16_t)0x0008
#define PL011_ICR_RXIC   (uint16_t)0x0010
#define PL011_ICR_TXIC   (uint16_t)0x0020
#define PL011_ICR_RTIC   (uint16_t)0x0040
#define PL011_ICR_FEIC   (uint16_t)0x0080
#define PL011_ICR_PEIC   (uint16_t)0x0100
#define PL011_ICR_BEIC   (uint16_t)0x0200
#define PL011_ICR_OEIC   (uint16_t)0x0400

#define PL011_DMACR_RXDMAE    (uint16_t)0x0001
#define PL011_DMACR_TXDMAE    (uint16_t)0x0002
#define PL011_DMACR_DMAAONERR (uint16_t)0x0004

#define PL011_UARTCLK_MIN (1420 * 1000UL)
#define PL011_UARTCLK_MAX (542720 * 1000UL)


// assume CLOCK is 1M
#define UART_CLOCK 1000000 //00
#define PL011_BASE_ADDR 0x9000000

int pl011_init(uint32_t baudrate)
{
    struct pl011_reg *reg = (struct pl011_reg *)PL011_BASE_ADDR;
    uint32_t clock_rate_x4;
    uint32_t divisor_integer = 0;
    uint32_t divisor_fractional = 4;
    uint32_t divisor;

    if (baudrate) {
        /* Ensure baud rate is not higher than the clock can support */
        clock_rate_x4 = (uint32_t)(UART_CLOCK * 4);

        /* Calculate integer and fractional divisors */
        divisor = clock_rate_x4 / baudrate;
        divisor_integer = divisor / 64;
        divisor_fractional = divisor % 64;
    }

    reg->IBRD = (uint16_t)divisor_integer;
    reg->FBRD = divisor_fractional;

    reg->ECR = PL011_ECR_CLR;
    reg->LCR_H = PL011_LCR_H_WLEN_8BITS | PL011_LCR_H_FEN;
    reg->CR = PL011_CR_UARTEN | PL011_CR_RXE | PL011_CR_TXE;

    return 0;
}


int pl011_putc(char ch)
{
    struct pl011_reg *reg = (struct pl011_reg *)PL011_BASE_ADDR;
#if 0    
    if ((reg->FR & PL011_FR_TXFF) > 0) {
        return ch;
    }

    reg->DR = (uint16_t)ch;
#else
    while ((reg->FR & PL011_FR_TXFF));
    reg->DR = (uint16_t)ch;
#endif
    return ch;
}


int pl011_getc(void)
{
    struct pl011_reg *reg = (struct pl011_reg *)PL011_BASE_ADDR;
#if 0    
     if (reg->FR & PL011_FR_RXFE) {
        return false;
    }
#else
    while ((reg->FR & PL011_FR_RXFE));
#endif
     return reg->DR;
}


void pl011_flush(void)
{
    struct pl011_reg *reg = (struct pl011_reg *)PL011_BASE_ADDR;
    while (reg->FR & PL011_FR_BUSY) {
 //       continue;
    } 
}