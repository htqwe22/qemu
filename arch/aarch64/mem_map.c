/**********************************************************************************
 * FILE : mem_map.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-05-26 , At 10:59:00
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "mem_map.h"
#include <arch_helpers.h>
#include <plat_def.h>
#include <stdint.h>
#include "log.h"
/*
 * reference registers:
 * HCR_EL2.{E2H, TGE}
 * SCR_EL3
 * TTBRn_ELX
 * TCR_ELx.TnSZ
 * ID_AARCH64MMFR2_ELx
 * VTCR_EL2.T0SZ (IPA)
 * MAIR_ELx (Memory Attribute Indirection Register)

*/

uint32_t get_pa_range(void)
{
    uint64_t val = read_id_aa64mmfr0_el1();
    // id_aa64mmfr0_el1.[3:0]
    uint32_t pa_range = (val >> ID_AA64MMFR0_EL1_PARANGE_SHIFT) & ID_AA64MMFR0_EL1_PARANGE_MASK;
    switch (pa_range) {
    case 0: return PARANGE_0000;
    case 1: return PARANGE_0001;
    case 2: return PARANGE_0010; //here is 40
    case 3: return PARANGE_0011;
    case 4: return PARANGE_0100;
    case 5: return PARANGE_0101;
    case 6: return PARANGE_0110; // needs PLA support
    default:
        return 0;
    }
}

uint32_t get_va_range(void)
{
    uint64_t val = read_id_aa64mmfr2_el1();
    // id_aa64mmfr2_el1.[19:16]
    uint32_t va_range = (val >> 16) & 0xf;
    switch (va_range) {
    case 0: return 48; //here is 48
    case 1: return 52; //needs LVA support
    default:
        return 0;
    }
}


void mem_map_init(void)
{
    uint64_t val;
    val = read_id_aa64mmfr0_el1();
    LOG_DEBUG("mmfr0: %016lx\n", val);
    val = read_id_aa64mmfr1_el1();
    LOG_DEBUG("mmfr1: %016lx\n", val);
    val = read_id_aa64mmfr2_el1();
    LOG_DEBUG("mmfr2: %016lx\n", val);

    val = read_tcr_el1();
    LOG_DEBUG("tcr_el1: %016lx\n", val);
    val = read_tcr_el2();
    LOG_DEBUG("tcr_el2: %016lx\n", val);
    val = read_tcr_el3();
    LOG_DEBUG("tcr_el3: %016lx\n", val);

    val = read_ttbr0_el1();
    LOG_DEBUG("ttbr0_el1: %016lx\n", val);
    val = read_ttbr0_el2();
    LOG_DEBUG("ttbr0_el2: %016lx\n", val);
    val = read_ttbr0_el3();
    LOG_DEBUG("ttbr0_el3: %016lx\n", val);

    val = read_midr_el1();
    LOG_DEBUG("midr_el1: %016lx\n", val);
    val = read_ttbr1_el1();
    LOG_DEBUG("ttbr1_el1: %016lx\n", val);

    val = read_mair_el1();
    LOG_DEBUG("mair_el1: %016lx\n", val);
    val = read_mair_el2();
    LOG_DEBUG("mair_el2: %016lx\n", val);
    val = read_mair_el3();
    LOG_DEBUG("mair_el3: %016lx\n", val);
    
    // val = read_ttbr1_el2();
    // LOG_DEBUG("ttbr1_el2: %016lx\n", val);


    // // val = read_vtcr_el1();
    // // LOG_DEBUG("vtcr_el1: %016lx\n", val);
    // val = read_vtcr_el2();
    // LOG_DEBUG("vtcr_el2: %016lx\n", val);

    // val = read_hcr_el2();
    // LOG_DEBUG("hcr_el2: %016lx\n", val);
    // val = read_scr_el3();
    // LOG_DEBUG("scr_el3: %016lx\n", val);
}

