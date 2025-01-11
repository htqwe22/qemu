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
#include <utils_def.h>
#ifdef __cplusplus
extern "C" {
#endif


#define SECTION_PAGE    __attribute__((section(".page_data")))
#define SECTION_DDR     __attribute__((section(".ddr_data")))

/*
 * Partition memory into secure ROM, non-secure DRAM, secure "SRAM",
 * and secure DRAM.
 */
#define SEC_ROM_BASE			0x00000000
#define SEC_ROM_SIZE			0x00020000

#define SEC_SRAM_BASE			0x0e000000
#define SEC_SRAM_SIZE			0x00100000

#define SEC_DRAM_BASE			0x0e100000  //used for far
#define SEC_DRAM_SIZE			0x00f00000

#define NS_DRAM0_BASE			0x40000000
#define NS_DRAM0_SIZE			0xC0000000

/*
 * Kevin defined memory map ...
 */
#define PLAT_DDR_BASE		NS_DRAM0_BASE
#define PLAT_DDR_SIZE		(2*GB) //NS_DRAM0_SIZE

#define STACK_SIZE          8*KB
#define STACK_BASE          (SEC_SRAM_BASE + SEC_SRAM_SIZE)

#define PAGE_SIZE           4096

#define configTOTAL_HEAP_SIZE   (200 * MB)

/**
 * Endof Kevin defined memory map...
*/


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
 * ARM-TF lives in SRAM, partition it here
 */

#define SHARED_RAM_BASE			SEC_SRAM_BASE
#define SHARED_RAM_SIZE			0x00001000

#define PLAT_QEMU_TRUSTED_MAILBOX_BASE	SHARED_RAM_BASE
#define PLAT_QEMU_TRUSTED_MAILBOX_SIZE	(8 + PLAT_QEMU_HOLD_SIZE)
#define PLAT_QEMU_HOLD_BASE		(PLAT_QEMU_TRUSTED_MAILBOX_BASE + 8)
#define PLAT_QEMU_HOLD_SIZE		(PLATFORM_CORE_COUNT * \
					 PLAT_QEMU_HOLD_ENTRY_SIZE)
#define PLAT_QEMU_HOLD_ENTRY_SHIFT	3
#define PLAT_QEMU_HOLD_ENTRY_SIZE	(1 << PLAT_QEMU_HOLD_ENTRY_SHIFT)
#define PLAT_QEMU_HOLD_STATE_WAIT	0
#define PLAT_QEMU_HOLD_STATE_GO		1

#define BL_RAM_BASE			(SHARED_RAM_BASE + SHARED_RAM_SIZE)
#define BL_RAM_SIZE			(SEC_SRAM_SIZE - SHARED_RAM_SIZE)

#define TB_FW_CONFIG_BASE		BL_RAM_BASE
#define TB_FW_CONFIG_LIMIT		(TB_FW_CONFIG_BASE + PAGE_SIZE)
#define TOS_FW_CONFIG_BASE		TB_FW_CONFIG_LIMIT
#define TOS_FW_CONFIG_LIMIT		(TOS_FW_CONFIG_BASE + PAGE_SIZE)

/*
 * BL1 specific defines.
 *
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 * Put BL1 RW at the top of the Secure SRAM. BL1_RW_BASE is calculated using
 * the current BL1 RW debug size plus a little space for growth.
 */
#define BL1_RO_BASE			SEC_ROM_BASE
#define BL1_RO_LIMIT			(SEC_ROM_BASE + SEC_ROM_SIZE)
#define BL1_RW_BASE			(BL1_RW_LIMIT - 0x12000)
#define BL1_RW_LIMIT			(BL_RAM_BASE + BL_RAM_SIZE)

/*
 * BL2 specific defines.
 *
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define BL2_BASE			(BL31_BASE - 0x35000)
#define BL2_LIMIT			BL31_BASE

/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE			(BL31_LIMIT - 0x60000)
#define BL31_LIMIT			(BL_RAM_BASE + BL_RAM_SIZE - FW_HANDOFF_SIZE)
#define BL31_PROGBITS_LIMIT		BL1_RW_BASE

#if TRANSFER_LIST
#define FW_HANDOFF_BASE			BL31_LIMIT
#define FW_HANDOFF_LIMIT		(FW_HANDOFF_BASE + FW_HANDOFF_SIZE)
#define FW_HANDOFF_SIZE			0x4000
#else
#define FW_HANDOFF_SIZE			0
#endif


/*
 * BL3-2 specific defines.
 *
 * BL3-2 can execute from Secure SRAM, or Secure DRAM.
 */
#define BL32_SRAM_BASE			BL_RAM_BASE
#define BL32_SRAM_LIMIT			BL31_BASE
#define BL32_DRAM_BASE			SEC_DRAM_BASE
#define BL32_DRAM_LIMIT			(SEC_DRAM_BASE + SEC_DRAM_SIZE - \
					 RME_GPT_DRAM_SIZE)

#define SEC_SRAM_ID			0
#define SEC_DRAM_ID			1

#if BL32_RAM_LOCATION_ID == SEC_SRAM_ID
# define BL32_MEM_BASE			BL_RAM_BASE
# define BL32_MEM_SIZE			BL_RAM_SIZE
# define BL32_BASE			BL32_SRAM_BASE
# define BL32_LIMIT			(BL32_SRAM_LIMIT - FW_HANDOFF_SIZE)
#elif BL32_RAM_LOCATION_ID == SEC_DRAM_ID
# define BL32_MEM_BASE			SEC_DRAM_BASE
# define BL32_MEM_SIZE			SEC_DRAM_SIZE
# define BL32_BASE			BL32_DRAM_BASE
# define BL32_LIMIT			(BL32_DRAM_LIMIT - FW_HANDOFF_SIZE)
#else
# error "Unsupported BL32_RAM_LOCATION_ID value"
#endif

#if TRANSFER_LIST
#define FW_NS_HANDOFF_BASE		(NS_IMAGE_OFFSET - FW_HANDOFF_SIZE)
#endif

#define NS_IMAGE_OFFSET			(NS_DRAM0_BASE + 0x20000000)
#define NS_IMAGE_MAX_SIZE		(NS_DRAM0_SIZE - 0x20000000)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_MMAP_REGIONS		(13 + MAX_MMAP_REGIONS_SPMC)
#define MAX_XLAT_TABLES			(6 + MAX_XLAT_TABLES_SPMC)
#define MAX_IO_DEVICES			4
#define MAX_IO_HANDLES			4

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
