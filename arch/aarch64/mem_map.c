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
#include <aarch64_mmu.h>
#include <stdint.h>
#include <heap.h>
#include <asm_func.h>
#include "log.h"
#include "stack_frame.h"



#define TABLE_NR    4096

SECTION_PAGE
struct page_table_4k tables[TABLE_NR];
SECTION_DDR
char aloc_flag[TABLE_NR];


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

uint32_t get_fix_pa_range(void)
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

uint32_t get_fix_va_range(void)
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


void debug_mmu_registers(void)
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

struct page_table_4k *alloc_page_table(void)
{
    int i = 0;
    for (i = 0; i < TABLE_NR; i++) {
        if (aloc_flag[i] == 0) {
            aloc_flag[i] = 1;
            return &tables[i];
        }
    }
    LOG_ERROR("alloc mmu table fail\n");
    uint64_t fp;
    asm volatile("mov %0, x29":"=r" (fp));
    debug_callstack((void *)fp);
    read_actlr_el1();
    for(;;);
    return NULL;
}

void free_page_table(struct page_table_4k *page_tabe)
{
#if 0    
    int i = 0;
    for (i = 0; i < TABLE_NR; i++) {
        if (&tables[i] == page_tabe) {
            aloc_flag[i] = 0;
            return;
        }
    }
#else
    aloc_flag[page_tabe - tables] = 0;
#endif
}

static inline struct page_table_4k *get_page_table(int el, uint64_t va)
{
    struct page_table_4k *pgd_table = NULL;
    switch(el) {
        case 3:
            pgd_table = (struct page_table_4k *)read_ttbr0_el3();
            break;
        case 2:
            pgd_table = (struct page_table_4k *)read_ttbr0_el2();
            break;
        case 1:
        case 0:
            if (va & 0xfff0000000000000) { // va >> 52
                pgd_table = (struct page_table_4k *)read_ttbr1_el1();
            } else {
                pgd_table = (struct page_table_4k *)read_ttbr0_el1();
            }
            break;
        default:
            break;
    }
    return pgd_table;
}

static inline void set_page_table(int el, uint64_t va, struct page_table_4k *pgd_table)
{
    // TODO check pgd_table != NULL
    switch(el) {
        case 3:
            write_ttbr0_el3((uint64_t)pgd_table);
            break;
        case 2:
            write_ttbr0_el2((uint64_t)pgd_table);
            break;
        case 1:
        case 0:
            if (va & 0xfff0000000000000) { // va >> 52
                write_ttbr1_el1((uint64_t)pgd_table);
            } else {
                write_ttbr0_el1((uint64_t)pgd_table);
            }
            break;
        default:
            return;
    }
}


void create_2m_mapping(int el, uint64_t pa, uint64_t va, uint64_t size, uint64_t attrs)
{
    struct page_table_4k *pgd_table = NULL;
    int contiguous = 0;
    pgd_table = get_page_table(el, va);
    if (pgd_table == NULL) {
        pgd_table = alloc_page_table();
        set_page_table(el, va, pgd_table);
    }
    // get pud entry in pgd & init pgd pattern
    for (; size >= 2*MB; size -= 2*MB) {
        table_4k_entry_t *pgd_pattern = &pgd_table->entries[(va >> 39) & 0x1ff];
        struct page_table_4k *next_table;
        if ((pgd_pattern->pgd.type & 1) == 0) {
            // allocate a new page table
            next_table = alloc_page_table();
            pgd_pattern->pgd.pud_base = (uint64_t)next_table >> 12;
            pgd_pattern->pgd.type = 0b11;
        }
        // now next table is pud table
        next_table = (struct page_table_4k *)((uint64_t)pgd_pattern->pgd.pud_base << 12);
        table_4k_entry_t *pud_pattern = &next_table->entries[(va >> 30) & 0x1ff];
        if ((pud_pattern->pud.type & 1) == 0) {
            // allocate a new page table
            next_table = alloc_page_table();
            pud_pattern->pud.pmd_base = (uint64_t)next_table >> 12;
            pud_pattern->pud.type = 0b11;
        }

        next_table = (struct page_table_4k *)((uint64_t)pud_pattern->pud.pmd_base << 12);
        table_4k_entry_t *pmd_pattern = &next_table->entries[(va >> 21) & 0x1ff];
        //if ((pmd_pattern->b2m.type & 1) == 0) 
        {
            pmd_pattern->b2m.type = 0b01; // block
            pmd_pattern->value |= 1 << 16; // block (bit 16 block only)
        }
        pmd_pattern->b2m.pa_base = pa >> 21;
        pa += 2*MB;
        va += 2*MB;
        pmd_pattern->value |= attrs;
        if (contiguous) {
            pmd_pattern->value |= (1ull << 52);
        }
        contiguous = 1;
    }
    asm volatile("tlbi vmalle1is");
    asm volatile("dsb ish");
    asm volatile("isb");
}



void create_4k_mapping(int el, uint64_t pa, uint64_t va, uint64_t size, uint64_t attrs)
{
    struct page_table_4k *pgd_table = NULL;
    int contiguous = 0;
    pgd_table = get_page_table(el, va);
    LOG_DEBUG("map va: %016lx, pa: %016lx, size: %d\n", va, pa, size);
    if (pgd_table == NULL) {
        pgd_table = alloc_page_table();
        set_page_table(el, va, pgd_table);
    }
    // get pud entry in pgd & init pgd pattern
    for (; size >= 4096; size -= 4096) {
        table_4k_entry_t *pgd_pattern = &pgd_table->entries[(va >> 39) & 0x1ff];
        struct page_table_4k *next_table;
        // Create pud table if not exist
        if ((pgd_pattern->pgd.type & 1) == 0) {
            // allocate a new page table
            next_table = alloc_page_table();
            pgd_pattern->pgd.pud_base = (uint64_t)next_table >> 12;
            pgd_pattern->pgd.type = 0b11;
        }
        // now next table is pud table
        next_table = (struct page_table_4k *)((uint64_t)pgd_pattern->pgd.pud_base << 12);
        table_4k_entry_t *pud_pattern = &next_table->entries[(va >> 30) & 0x1ff];
        // Create pmd table if not exist
        if ((pud_pattern->pud.type & 1) == 0) {
            // allocate a new page table
            next_table = alloc_page_table();
            pud_pattern->pud.pmd_base = (uint64_t)next_table >> 12;
            pud_pattern->pud.type = 0b11;
        }

        next_table = (struct page_table_4k *)((uint64_t)pud_pattern->pud.pmd_base << 12);
        table_4k_entry_t *pmd_pattern = &next_table->entries[(va >> 21) & 0x1ff];
        // Create pte table if not exist
        if ((pmd_pattern->pte.type & 1) == 0) {
           // allocate a new page table
            next_table = alloc_page_table();
            pmd_pattern->pmd.pte_base = (uint64_t)next_table >> 12;
            pmd_pattern->pmd.type = 0b11;
        }

        next_table = (struct page_table_4k *)((uint64_t)pmd_pattern->pmd.pte_base << 12);
        table_4k_entry_t *pte_pattern = &next_table->entries[(va >> 12) & 0x1ff];
     //   if ((pte_pattern->pte.type & 1) == 0)  force setting ...
        {
            pte_pattern->pte.type = 0b11; // page
        }
        pte_pattern->pte.pa_base = pa >> 12;
        pte_pattern->value |= attrs;
        if (contiguous) {
            pte_pattern->value |= (1ull << 52);
        }
        contiguous = 1;
        pa += 4096;
        va += 4096;
    }
    // 刷新TLB
    asm volatile("tlbi vmalle1is");
    asm volatile("dsb ish");
    asm volatile("isb");

    // // 刷新数据缓存
    // asm volatile("dc ivac, %0" : : "r" (address));
    // asm volatile("dsb ish");
}

extern char ddr_data_start[];

void mem_map_init(void)
{
    uint32_t cur_el = get_current_el();
    uint32_t pa_range = get_fix_pa_range();
    uint32_t va_range = get_fix_va_range();
    LOG_INFO("Support PA bits: %d\n", pa_range);
    LOG_INFO("Support VA bits: %d\n", va_range);
    LOG_DEBUG("Configuring MMU for EL%d ...\n", cur_el);
    init_mmu_elx(cur_el);
    memset_64(tables, 0, sizeof(tables));
    memset_64(aloc_flag, 0, sizeof(aloc_flag));

 #if 0
    create_2m_mapping(cur_el, SEC_ROM_BASE, SEC_ROM_BASE, 2*MB, ATTR_MEM_RO_EXE); // for .text
    create_2m_mapping(cur_el, SEC_SRAM_BASE, SEC_SRAM_BASE, 2*MB, ATTR_MEM_NORMAL); // for .stack
 //   create_2m_mapping(cur_el, PLAT_DDR_BASE, PLAT_DDR_BASE, PLAT_DDR_SIZE, ATTR_MEM_NORMAL);
 //   create_2m_mapping(cur_el, (uint64_t)ddr_data_start, (uint64_t)ddr_data_start, 1*GB, ATTR_MEM_NORMAL);
    create_2m_mapping(cur_el, UART0_BASE, UART0_BASE, 2*MB, ATTR_DEV_NE);
#else
    create_4k_mapping(cur_el, SEC_ROM_BASE, SEC_ROM_BASE, SEC_ROM_SIZE, ATTR_MEM_RO_EXE); // for .text
    create_4k_mapping(cur_el, SEC_SRAM_BASE, SEC_SRAM_BASE, SEC_SRAM_SIZE, ATTR_MEM_NORMAL); // for .stack
    create_4k_mapping(cur_el, PLAT_DDR_BASE, PLAT_DDR_BASE, 512*MB, ATTR_MEM_NORMAL);
    create_2m_mapping(cur_el, UART0_BASE, UART0_BASE, 2*MB, ATTR_DEV_NE);
#endif

#if 0
    #define TEST_SEC_BASE (SEC_DRAM_BASE + 1*MB)
    *(uint64_t *)(TEST_SEC_BASE) = 0x12345678;
    create_4k_mapping(cur_el, TEST_SEC_BASE, 0x200001000UL, 4*KB, ATTR_MEM_NORMAL);
#endif
    LOG_INFO("MMU configure done\n");
    enable_mmu_elx(cur_el);
    LOG_INFO("MMU works done\n");
 //   LOG_DEBUG("sec dram %x\n", *(uint64_t *)(0x200001000UL));
    #if 0 // try to map transform table
    create_2m_mapping(cur_el, PLAT_DDR_BASE, PLAT_DDR_BASE, (uint64_t)ddr_data_start - PLAT_DDR_BASE, ATTR_MEM_NORMAL);
    LOG_INFO("can not be here\n");
    #endif
}

