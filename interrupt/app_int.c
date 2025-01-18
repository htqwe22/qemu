/**********************************************************************************
 * FILE : app_int.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-12 , At 15:26:34
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "app_int.h"
#include <gicv3_basic.h>
#include <gicv3_lpis.h>
#include <asm_func.h>
#include <plat_def.h>
#include <arch_helpers.h>
#include <log.h>
#include <mem_map.h>

// rd_base， 其中rd表示redistributor




void gic_global_init(int fiq_in_el3, int irq_in_el3, int external_abort_in_el3)
{
    // 找到GICD GICR
    setGICAddr((void*)GICD_BASE, (void*)GICR_BASE);
    // 通过读这个寄存器知道GICD的地址已经对的（0x43b）
    LOG_DEBUG("GICD_IIDR: %x\n", gic_dist->GICD_IIDR);
    // Enable ARE & Group NO 1-of-N
    enableGIC();
    debug_gicd();

    //配置全局的SCR_EL3中断路由
    //SCR_EL3.IRQ/FIQ针对物理的IRQ/FIQ的配置
    // 0 表示PE在低于EL3的级别下执行时物理的中断不会路由到EL3,而且在EL3下执行时物理的相应中断不会被接受（taken）
    // 1 任何中断都会被EL3接受.
    // SCR_EL3.EA: 外部中止和SError的路由
    // 0: 表示在低于EL3的级别下执行外部中止和SError不会路由到EL3,而且在EL3下执行时外部只有中止才能被EL3处理
    // 1  任何外部中止和SError都由EL3处理。
    // 从上可知，外部中止会一定会被路由到EL3.
    // SER_EL3.WR = 1，表示低于EL3级别的PE处于AARCH64执行状态。
    uint64_t scr_el3 = read_scr_el3();
    scr_el3 |= SCR_RW_BIT;
    if (irq_in_el3)
        scr_el3 |= SCR_IRQ_BIT;
    else
        scr_el3 &= ~SCR_IRQ_BIT;

    if (fiq_in_el3)
        scr_el3 |= SCR_FIQ_BIT;
    else
        scr_el3 &= ~SCR_FIQ_BIT;

    if (external_abort_in_el3)
        scr_el3 |= SCR_EA_BIT;
    else
        scr_el3 &= ~SCR_EA_BIT;

    write_scr_el3(scr_el3);
}


void gic_current_pe_init(void)
{
    uint32_t rd;
//    LOG_DEBUG("cur affinity: %d.%d.%d.%d\n", affi>>24, (affi>>16)&0xff, (affi>>8)&0xff, affi&0xff);
    //通过这里可以看出当前在EL3下实现了4bit的优先级。
    LOG_DEBUG("priority support bits: %d\n", get_fixed_Priority_bits());
    getGICRTyper();

  //  getGICRTyper();
    // 根据亲和性匹配到 redistributor ID
    rd = getRedistID(getAffinity());
    // wakeup本PE对应的redistributor ...
    wakeUpRedist(rd);
    
    // 配置CPU interface（好像很复杂，有GICC和ICC的寄存器）
    // 看文档，在复位时设定SRE=0，只能使用GICC的寄存器来配置CPU interface ...
    // 但仿真没有遇到问题，先读一下这个寄存器的值吧
    uint32_t ser = getICC_SRE_EL3();
    LOG_DEBUG("ICC_SRE_EL3: %x, %x, %x\n", ser, getICC_SRE_EL2(), getICC_SRE_EL1());
    // Set SER = 1 to access ICC_* registers

    // control register ICC_CTLR_EL3
    uint32_t icc_ctrl = (0x1 << 6); //PMHE = 1;
    setICC_CTLR_EL3(icc_ctrl);
 // control register ICC_CTLR_EL2 (NO EL2) 

     // control register ICC_CTLR_EL1
     /**
      * CBPR = 0, EOI = 0, PMHE = 1, 
     */
    icc_ctrl = 1 << 6;
    setICC_CTLR_EL1(icc_ctrl);

    //here we can use ICH_* & ICC_* to access CPU Interface registers 
    setPriorityMask(0xFF); // 优先级屏蔽寄存器,这里设置为最低，表示不屏蔽

    enableGroup0Ints();
    enableGroup1Ints();
    // ICC_IGRPEN1_EL3 can also be used to enable interrupts for NS & S group 1
    // This call only works as example runs at EL3
    enableNSGroup1Ints(); 

    /**
     *  Binary point value Group priority field Subpriority field Field with binary point
     *    0                 [7:1]                 [0]                 ggggggg.s
     *    1                 [7:2]               [1:0]                 gggggg.ss
     */
    //低两位是子优先级，高6位是组优先级
    setBPR0(1);  // for group 0
    // ICC_BPR1_EL1 This register is banked between ICC_BPR1_EL1 and ICC_BPR1_EL1_S and ICC_BPR1_EL1_NS
    setBPR1(1);  // for group 1 SECURITY STATE in EL3 
}



/**
 * trigger & target_affi is only valid for SPI
*/
int gic_configure_interrupt(int INTID, uint8_t priority, int_group_t group, irq_trigger_t trigger, uint32_t target_affi, irq_handler_t handler, void *priv_arg)
{
    if (handler == NULL)
        return -1;
    local_register_irq_handler(INTID, handler, priv_arg);
    // get the correct redistributor ID
    if (target_affi == 0xffffffff) {
        target_affi = getAffinity();
    }
    uint32_t rd = getRedistID(target_affi);
    // set the interrupt priority
    if (setIntPriority(INTID, rd, priority)) {
        LOG_ERROR("failed to set priority for INTID: %d\n", INTID);
        local_unregister_irq_handler(INTID);
        return -2;
    }
    // set Trigger Mode // 0b00 LEVEL, 0b10, EDGE
    setIntType(INTID, rd, trigger);
    // set secure state & group; together with Group Modfier & Group secure status
    /** GICD_IGRPMODR   GICD_IGROUPR    Definition
     *   0              0              0: G0S
     *   0              1              1: G1NS
     *   1              0              2: G1S
    */
    if (setIntGroup(INTID, rd, group)) {
        LOG_ERROR("failed to set group for INTID: %d\n", INTID);
        local_unregister_irq_handler(INTID);
        return -3;  
    }

    if ((INTID > 31  && INTID < 1020) || 0 /*E-SPI*/) {
        // SPI set Target PE
        // mode: (0<<31) use A3.A2.A1.A0; (1<<31) use 1-of-N
        if (setIntRoute(INTID, 0, target_affi)) {
            LOG_ERROR("failed to set route for INTID: %d\n", INTID);
            local_unregister_irq_handler(INTID);
            return -4;  
        }
    }
    
    // enable the interrupt
    if (enableInt(INTID, rd)) {
        LOG_ERROR("failed to enable INTID: %d\n", INTID);
        local_unregister_irq_handler(INTID);
        return -5;  
    }
    return 0;
}

int gic_set_interrupt_pending(int INTID, uint32_t target_affi)
{
    uint32_t rd = getRedistID(target_affi);
    return setIntPending(INTID, rd);
}



int gic_lpi_init(uint32_t rd, uint32_t lpi_id_num)
{
    //单个的LPI的空间大小由GICR_PROPBASER.ID来决定，整个LPI配置的最大值由GICD_TYPER.IDbits来决定
    // 以上说的是一回事，LPI的配置大小的上限受限于GICD
    uint32_t gicd_typer = getGICDTyper();
    uint32_t max_id_bits = ((gicd_typer >> 19) & 0x1f) + 1;
 //   uint32_t max_id = 1u << (max_id_bits + 1);
    uint32_t id_bits;

    // Get the minimum id_bits
    for (id_bits = 14; id_bits <= max_id_bits; id_bits++) {
        if (( 1 << id_bits ) >= (lpi_id_num + 8192))
            break;
    }
    if ((1 << id_bits ) > (lpi_id_num + 8192)) {
        LOG_ERROR("id_bits reaches ID bits define in GICD\n");
        return -1;
    }
   // lpi_id_num = (1ull << id_bits) - 8192;

    uint32_t num = 1;
    // 4KB is large enough.
    void *p_pending_table = alloc_page_table_aligned(PAGE_ALIGN_64K, num);
    // clear the pending table
    uint64_t tmp;
    // cause eatch lpi tasks only one bits.
    tmp = 1ull << (id_bits - 3); 
    memset(p_pending_table, 0, tmp);

    //set GICR_PENDBASER
    // 0b110 cacheable, write-allocate, read-allocate, write-through
    // 0b01 at bit10; Inner Shareable
    tmp = (0b110ull << 7) | (0b01ull << 10) | (uint64_t)p_pending_table; // & 0x0000FFFFFFFF0000
    // setLPIPendingTableAddr;
    gic_rdist[rd].lpis.GICR_PENDBASER = tmp;

    // set GICR_PROPBASER for eatch rd. and use the same prop_table.
    static void *sh_prop_table = NULL;
    if (sh_prop_table == NULL) {
        tmp = 4096;
        for (; tmp < lpi_id_num; tmp += 4096);
        sh_prop_table = alloc_page_table_aligned(PAGE_ALIGN_4K, tmp >> 12);
        // cause eatch lpi tasks one byte.
        memset(sh_prop_table, 0, lpi_id_num);
    }
    // Outer & Inner Shareable, inner share.
    tmp = (0b110ull << 56) | (0b110ull << 7) | (0b10ull << 10) | (uint64_t)sh_prop_table | \
            ((id_bits - 1) & 0x1f); // & 0x0000FFFFFFFF0000
    gic_rdist[rd].lpis.GICR_PROPBASER = tmp;
    //Note: latter, we will set the priority and enable bits in the prop_table when we use.
    // enable lpi 
    gic_rdist[rd].lpis.GICR_CTLR |= 1;
    return 0;
}

int gic_its_init(uint32_t rd, uint32_t lpi_id_num)
{
    setITSBaseAddress((void *)GICI_BASE);
    uint64_t its_typer = debug_its_typer();
    uint32_t devid_bits = ((its_typer >> 13) & 0x1f) + 1;
    uint32_t evid_bits = ((its_typer >> 8) & 0x1f) + 1;

    // 1. Check the right table for Device Table 
    uint64_t tmp;
    int idx;
    //here page size maybe RO.
    uint32_t page_size = 0; 

    uint32_t dev_idx = 0xff, collection_idx = 0xff;
    uint32_t dev_entry_size, collection_entry_size;
    uint32_t type, entry_size;
    for (idx = 0; idx < 8; idx++) {
        getITSTableType(idx, &type, &entry_size);
        LOG_DEBUG("idx %d, type %d, pg_size %d, entry_size %d\n", idx, type, (gic_its->GITS_BASER[idx] >> 8) & 3, entry_size);
        if (type == 1) {
            dev_idx = idx;
            dev_entry_size = entry_size + 1;
        }else if (type == 4) {
            collection_idx = idx;
            collection_entry_size = entry_size + 1;
        }
        if (dev_idx != 0xff && collection_idx != 0xff)
            break;
    }
    if (idx == 8) {
        LOG_ERROR("get its table type failed\n");
        return -1;
    }

    void *table;
    uint64_t config_value;
    uint32_t size;
    // size is 2^(DTE.ITTRange + 1)* GITS_TYPER.ITT_entry_size
    // ITT Range Log2 (EventID width supported by the ITT) minus one
    // now ITT_RANGE = log2(16) = 4;
    size = (((its_typer >> 4) & 0x1f) + 1) * 4;
    size = (size + 4095) >> 12 - 1;
    table = alloc_page_table_aligned(PAGE_ALIGN_64K, size);
    memset(table, 0, size * 4096);
    // Command queue table ...
    config_value = (1ull << 63) | (0b110ull << 59) | (0b110ull << 53) \
         |(0b10 << 10) |  (size -1) | (uint64_t)table;
    write64_separately(&gic_its->GITS_CBASER, config_value);
    gic_its->GITS_CWRITER = 0;    // This register contains the offset from the start, hence setting to 0
    dsb();

    // FOR DEV table
    // page size is 4k
    // page num = 0
    // Outer share  2 << 10;
    // 
    // TODO: let page size = 0 or page_size = 1; or page_size = 2;

    size = 1;
#if 0   // we just use 4k for now.
    uint32_t table_size = (1ull << evid_bits) * dev_entry_size;
    for (size = 4096; size < table_size; size += 4096);
    size >>= 12; // /4K
#else
    (void)devid_bits;
    (void)dev_entry_size;    
#endif
    switch (page_size)
    {
    case 0:
        table = alloc_page_table_aligned(PAGE_ALIGN_4K, size);
        break;
    case 1:
        table = alloc_page_table_aligned(PAGE_ALIGN_16K, size);
        break;
    case 2:
        table = alloc_page_table_aligned(PAGE_ALIGN_16K, size);
        break;
    default:
        LOG_ERROR("PAGE sie wrong %d\n", page_size);
       return -2;
    }

    memset(table, 0, size * 4096);
    size -= 1;
    // single level, normal cache. outer sheareable
    tmp = (1ull << 63) | (0ull << 62) | (0b110ull << 59) | (0b110ull << 53) \
         |(0b10 << 10) |  (page_size << 8) ;
    config_value = tmp | ((uint64_t)table | size);
    write64_separately(&gic_its->GITS_BASER[dev_idx], config_value);

    //configure collection table
    size = 1;
#if 0   // we just use 4k for now.
    uint32_t table_size = (1ull << devid_bits) * dev_entry_size;
    for (size = 4096; size < table_size; size += 4096);
    size >>= 12; // /4K
#else
    (void)evid_bits;
    (void)collection_entry_size;    
#endif
    switch (page_size)
    {
    case 0:
        table = alloc_page_table_aligned(PAGE_ALIGN_4K, size);
        break;
    case 1:
        table = alloc_page_table_aligned(PAGE_ALIGN_16K, size);
        break;
    case 2:
        table = alloc_page_table_aligned(PAGE_ALIGN_16K, size);
        break;
    default:
        LOG_ERROR("PAGE sie wrong %d\n", page_size);
       return -2;
    }
    memset(table, 0, size * 4096);
    size -= 1;
    // tmp = (1ull << 63) | (0ull << 62) | (0b110ull << 59) | (0b110ull << 53) 
    //     | (uint64_t)table |(0b10 << 10) |  (page_size << 8) | size;
    config_value = tmp | ((uint64_t)table | size);
    write64_separately(&gic_its->GITS_BASER[collection_idx], config_value);
    return 0;
}


int gicv3_spi_enable(uint32_t INTID, )



int config_lpi_int(uint32_t rd, uint32_t intid, uint32_t priority)
{
   configureLPI(rd, intid, 1, priority);
}