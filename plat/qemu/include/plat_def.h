/**********************************************************************************
 * FILE : plat/qemu/include/plat_def.h
 * Description:
 * Author: Kevin He
 * Created On: 2024-12-23 , At 19:44:55
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_PLAT_DEF_H
#define KV_PLAT_DEF_H
#include <plat_common.h>
#ifdef __cplusplus
extern "C" {
#endif





/*
 * Platform memory map
 */
#define PLAT_DDR_BASE		0x40000000
#define PLAT_DDR_SIZE		G_(2)

#define STACK_SIZE          M_(1)
#define STACK_BASE          (PLAT_DDR_BASE + 4 * STACK_SIZE)

#define TTBR_BASE	        STACK_BASE
#define TTBR_SIZE	        M_(100)
#define PAGE_SIZE           K_(4)

#define MEMORY_USE_BADDR	TTBR_BASE + TTBR_SIZE
#define MEMORY_USER_SIZE    (PLAT_DDR_SIZE - (MEMORY_USE_BADDR - PLAT_DDR_BASE))

#define configTOTAL_HEAP_SIZE   M_(200)

#if !defined __LINKER__
#if TTBR_BASE & 0xfff
#error TTBR_BASE must be aligned to PAGE_SIZE
#endif
#endif

/*
 * Partition memory into secure ROM, non-secure DRAM, secure "SRAM",
 * and secure DRAM.
 */
#define SEC_ROM_BASE			0x00000000
#define SEC_ROM_SIZE			0x00020000

#define NS_DRAM0_BASE			(0x40000000ULL)
#define NS_DRAM0_SIZE			(0xc0000000ULL)

#define SEC_SRAM_BASE			0x0e000000
#define SEC_SRAM_SIZE			0x00100000

#define SEC_DRAM_BASE			0x0e100000
#define SEC_DRAM_SIZE			0x00f00000

#define SECURE_GPIO_BASE		0x090b0000
#define SECURE_GPIO_SIZE		0x00001000
#define SECURE_GPIO_POWEROFF	0
#define SECURE_GPIO_RESET		1

/* Load pageable part of OP-TEE 2MB above secure DRAM base */
#define QEMU_OPTEE_PAGEABLE_LOAD_BASE	(SEC_DRAM_BASE + 0x00200000)
#define QEMU_OPTEE_PAGEABLE_LOAD_SIZE	0x00400000

/*
 * ARM-TF lives in SRAM, partition it here
 */

#define SHARED_RAM_BASE			SEC_SRAM_BASE
#define SHARED_RAM_SIZE			0x00001000

/*
 * PL011 related constants
 */
#define UART0_BASE			0x09000000
#define UART1_BASE			0x09040000
#define UART0_CLK_IN_HZ			1
#define UART1_CLK_IN_HZ			1

#define QEMU_FLASH0_BASE		0x00000000
#define QEMU_FLASH0_SIZE		0x04000000
#define QEMU_FLASH1_BASE		0x04000000
#define QEMU_FLASH1_SIZE		0x04000000

#define PLAT_QEMU_FIP_BASE		0x00040000
#define PLAT_QEMU_FIP_MAX_SIZE		(QEMU_FLASH0_SIZE - PLAT_QEMU_FIP_BASE)

#define DEVICE0_BASE			0x08000000
#define DEVICE0_SIZE			0x01000000
#define DEVICE1_BASE			0x09000000
#define DEVICE1_SIZE			0x00c00000

/*
 * GIC related constants
 */

#define GICD_BASE			0x8000000
#define GICC_BASE			0x8010000
#define GICR_BASE			0x80A0000


#define QEMU_IRQ_SEC_SGI_0		8
#define QEMU_IRQ_SEC_SGI_1		9
#define QEMU_IRQ_SEC_SGI_2		10
#define QEMU_IRQ_SEC_SGI_3		11
#define QEMU_IRQ_SEC_SGI_4		12
#define QEMU_IRQ_SEC_SGI_5		13
#define QEMU_IRQ_SEC_SGI_6		14
#define QEMU_IRQ_SEC_SGI_7		15

/*
 * DT related constants
 */
#define PLAT_QEMU_DT_BASE		NS_DRAM0_BASE
#define PLAT_QEMU_DT_MAX_SIZE		0x100000

/*
 * Platforms macros to support SDEI
 */
#define PLAT_PRI_BITS			    U(3)
#define PLAT_SDEI_CRITICAL_PRI		0x60
#define PLAT_SDEI_NORMAL_PRI		0x70
#define PLAT_SDEI_SGI_PRIVATE		QEMU_IRQ_SEC_SGI_0


#ifdef __cplusplus
}
#endif

#endif //plat/qemu/include/plat_def.h
