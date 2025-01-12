/**********************************************************************************
 * FILE : aarch64_mmu.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-01 , At 19:01:44
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "aarch64_mmu.h"
#include "arch_helpers.h"


void init_mmu_elx(int n_el)
{
	uint64_t temp_value = 0;
	// MAIR_EL3
	const uint64_t mair_value = 
		  MAIR_ATTR_SET(MAIR_DEVICE_nGnRnE, MAIR_DEVICE_nGnRnE_IDX) 
		| MAIR_ATTR_SET(MAIR_DEVICE_nGnRE, MAIR_DEVICE_nGnRE_IDX)
		| MAIR_ATTR_SET(MAIR_DEVICE_nGRE, MAIR_DEVICE_nGRE_IDX)
		| MAIR_ATTR_SET(MAIR_DEVICE_GRE, MAIR_DEVICE_GRE_IDX)
		| MAIR_ATTR_SET(MAIR_MEM_NORMAL, MAIR_MEM_NORMAL_IDX)
		| MAIR_ATTR_SET(MAIR_MEM_NC, MAIR_MEM_NC_IDX)
		| MAIR_ATTR_SET(MAIR_MEM_WT, MAIR_MEM_WT_IDX)
		| MAIR_ATTR_SET(MAIR_MEM_WB, MAIR_MEM_WB_IDX);
	switch (n_el) {
	case 3:
		write_mair_el3(mair_value);
		//TCR_EL3
		// DS = 0 未使用52bits的输出地址
		// TCMA = 0，使能未检查访问，1表示不做检查
		// TBID = 1，控制TBI的的地址匹配是否应用到指令和数据访问，1表示只影响数据访问
		// HWU62 = 0，第一阶段的块和页表项中的bit62不被硬件使用，1表示使用使用，具体怎么使用参考HPD说明
		// HWU61 ... 59 = 0, 同上。说明硬件在块和页表项中的[62:59]这四个bit会被硬件使用，FEAT_HPDS2
		// HPD （FEAT_HPDS） = 0,  使能分层的权限控制(PBHA[3:0])，1表示禁用
		// ｛HD, HA｝= 00， Stage 1 Access flag update enabled，硬件自动管理dirty状态,00表示由软件维护
		// TBI = 0， 高8位的虚拟地址是否用于地址计算，0表示被使用到，1表示会被忽略，
		// PS= 2. 40bits(这里我设置成和ID_AA64MMFR0_EL1_PARANG中的值一样))
		// TG0 = 2，使用16K的页表。
		// SH0 = 3，内部共享
		// ORGN0 = 0，Outer 不共享
		// IRGN0 = 0，Inner共享能力为不共享
		// T0SZ = 16
		temp_value = TCR_EL3_RES1
			| (1L << 29) 	//TBI only for data access
			| (1 << 20) 	//TBI enable
			| (1 << 24) 	//HPD disable, APTable, PXNTable,and UXNTable, except NSTable be ignored by PE
			| (0b11 << 21) 	//HD, HA 硬件管理dirty状态
			| (TCR_PS_BITS_1TB << 16) //PS = 40bit
			| TCR_TG0_4K    // Granule size 4K
			| (3 << 12) 	// SH0 Inner Shareable.
			| TCR_RGN_OUTER_NC 	// ORGN0 Outer Non-shareable
			| TCR_RGN_INNER_WBA  // IRGN0 Inner shareable high performance
			| 24 	// T0SZ, VA of (64 - 24 = 40) bits.
			;
		write_tcr_el3(temp_value);
		write_ttbr0_el3(0);
	break;
	case 2:
		write_mair_el2(mair_value);
	break;
	case 1:
	case 0:  // levl 1 & 0 use the same tcr_el1
		write_mair_el1(mair_value);
		//TODO ...
		write_tcr_el1(temp_value);
		write_ttbr0_el1(0);
		write_ttbr1_el1(0);
	break;
	break;
	default :
	return;
	}
}

void enable_mmu_elx(int n_el)
{
	uint64_t sctlr_elx;
	// memory that writeable teated as Execute-never
	switch (n_el) {
	case 3:
		sctlr_elx = read_sctlr_el3();
		sctlr_elx |= (SCTLR_M_BIT | SCTLR_WXN_BIT);
		write_sctlr_el3(sctlr_elx);
	break;	
	case 2:
		sctlr_elx = read_sctlr_el2();
		sctlr_elx |= (SCTLR_M_BIT | SCTLR_WXN_BIT);
		write_sctlr_el2(sctlr_elx);
		break;
	case 1:
	case 0:
		sctlr_elx = read_sctlr_el1();
		sctlr_elx |= (SCTLR_M_BIT | SCTLR_WXN_BIT);
		write_sctlr_el2(sctlr_elx);
	break;
	}
	isb();	/* ensure MMU is off */
	dsbsy();
}
