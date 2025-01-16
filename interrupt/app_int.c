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

/* 初始化流程可以参考文档中4章节,初始化的先后顺序和内容都涉及了
 * 根据框架图: Affinity Routing Enable
 * 全局配置
 * 整个SOC就一个Distributor和多个CPU Interface与Redistributor.
   1. 先查到当前SOC共有多少个Redistributor （GICR_TYPER, Redistributor Type Register查看是否是最后一个）
   2. 要使用ARE功能、Group的功能、1-of-N功能都在GICD_CTLR中定义
   3. 注意写AER有要求group的en都为0，如果再想写group的en，需要再写group的控制位
    ****************************
   单PE配置
   1. 唤醒PE
   2. CPU interface配置( ICC_*)
   3. 使能中断的group组功能 ICC_IGRPEN0_EL1/ICC_IGRPEN1_EL1
   4. 配置中断优先级（ICC_PMR_EL1）和 优先级的分组（ICC_BPR0_EL1/ICC_BPR1_EL1）
   5. 设置 EOI （EndOfInterrupt） ICC_CTLR_EL1/ ICC_CTLR_EL3 
 * 
 */

struct interrupt_handler_map
{
    uint32_t intid;
    irq_handler_t handler;
    void *arg;
};

static struct interrupt_handler_map int_handler_map[10];

void user_irq_handler(uint32_t INITID)
{
    for (int i = 0; i < ARRAY_SIZE(int_handler_map); i++) {
        if (int_handler_map[i].intid == INITID) {
            int_handler_map[i].handler(int_handler_map[i].arg);
            return;
        }
    }
    LOG_ERROR("intid: %d never been registered\n", INITID);
}

static int local_register_irq_handler(uint32_t intid, irq_handler_t handler, void *arg)
{
    // register interrupt handler ...
    // check if the interrupt is already configuredin
    for (int i = 0; i < ARRAY_SIZE(int_handler_map); i++) {
        if (int_handler_map[i].intid == intid) {
            return -1;
        }
    }
    for (int i = 0; i < ARRAY_SIZE(int_handler_map); i++) {
        if (int_handler_map[i].handler == NULL) {
            int_handler_map[i].handler = handler;
            int_handler_map[i].intid = intid;
            int_handler_map[i].arg = arg;
            return 0;
        }
    }
    return -2;
}

static int local_unregister_irq_handler(uint32_t intid) 
{
    for (int i = 0; i < ARRAY_SIZE(int_handler_map); i++) {
        if (int_handler_map[i].intid == intid) {
            int_handler_map[i].handler = NULL;
            int_handler_map[i].intid = 0;
            return 0;
        }
    }
    // not found
    return -1;
}


void write64_separately(volatile uint64_t *addr, uint64_t val)
{
    volatile uint32_t *ptr = (volatile uint32_t *)addr;
    *ptr++ = (val >> 32)& 0xffffffff;
    *ptr = val & 0xffffffff;
}

uint64_t read64_separately(volatile uint64_t *addr)
{   
   volatile uint32_t *ptr = (volatile uint32_t *)addr;
   return ((uint64_t)*ptr << 32) | *(ptr + 1);
}

void debug_gicd(void)
{
    uint32_t gicd_typer = gic_dist->GICD_TYPER;
    uint32_t tmp, tmp1;
    int e_spi_support = gicd_typer & 0x100; //(gicd_typer >> 8) & 1;

    LOG_DEBUG("==================== GICD =======================\n");
    LOG_DEBUG("GICD_CTLR: %x\n", gic_dist->GICD_CTLR);
    LOG_DEBUG("SPI num: %d\n", ((gicd_typer & 0x1f) + 1) * 32);
    LOG_DEBUG("Extended SPI num: %d\n", e_spi_support ? (((gicd_typer >> 27) & 0x1F) + 1) * 32 : 0);
    LOG_DEBUG("Non-maskable Interrupt support: %s\n", gicd_typer & (1<<9) ? "yes":"no"); 
    LOG_DEBUG("GICD support: %d security state\n", gicd_typer & (1<<10) ? 2:1);
    
    // debug LPI
    if (gicd_typer & (1u << 17)) {
      tmp = (gicd_typer >> 11) & 0x1f; 
      if (tmp == 0) {
        tmp1 = (gicd_typer >> 19) & 0x1f;
      } else {
        tmp1 = (1ULL << (tmp + 1)) - 1;
      }
    }else{
      tmp1 = 0;
    }
    LOG_DEBUG("LPI num: %u\n", tmp1); 
    LOG_DEBUG("write to GICD creates message-based irq support: %s\n", gicd_typer & (1u << 16) ? "yes":"no"); 
    LOG_DEBUG("Affinity 3 valid: %s\n", gicd_typer & (1u << 24) ? "yes":"no"); 
    LOG_DEBUG("1 of N SPI support: %s\n", gicd_typer & (1u << 25) ? "no":"yes");
    LOG_DEBUG("IRI supports targeted SGIs with affinity level 0: %s\n", gicd_typer & (1u << 26) ? "0 - 255":"0 - 15"); 
}


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


uint64_t debug_its_typer(void)
{
    uint64_t its_typer = gic_its->GITS_TYPER;
    uint32_t tmp;
    LOG_DEBUG("============== ITS_TYPER ==================\n");
    LOG_DEBUG("INV(cache disable when invalid): %d\n", (its_typer >> 46) & 1);
    LOG_DEBUG("UMSIirq support: %d\n", (its_typer >> 45) & 1);
    LOG_DEBUG("UMSI: %d\n", (its_typer >> 44) & 1);
    tmp = 16;
    if (its_typer & (1ul << 36)) {
        tmp = (its_typer >> 32) & 0xf;
    }
    LOG_DEBUG("CLI(Collection ID size ): %d\n",tmp);
    LOG_DEBUG("HCC(Hardware Collection Count): %d\n",(its_typer >> 24) & 0xff);
    LOG_DEBUG("PTA(Physical Target Addresses defined by): %s\n", its_typer & (1ul << 19) ? "physical address":"PE Number");
    LOG_DEBUG("Devbits(DeviceID bits): %d\n", ((its_typer >> 13) & 0x1f) + 1);
    LOG_DEBUG("ID_bits(EventID bits): %d\n", ((its_typer >> 8) & 0x1f) + 1);
    LOG_DEBUG("ITT_entry_size(transfer entry): %d\n", ((its_typer >> 4) & 0x1f) + 1);
    LOG_DEBUG("CCT(Cumulative Collection Tables): %d (choice)\n", (its_typer >> 2) & 1);
    LOG_DEBUG("supports physical LPI : %d (choice)\n", its_typer & 1);
    return its_typer;
}

/** UM MSI,是指没有通过ITS而发送的MSI。MSI可以映射为（一般UM MSI）SPI，也可映射为（通过ITS转换成）LPI
 * 注意64位的寄存器有可能要求是按2个32位来访问的，文档会有描述
 * 表的配置有(0-7共8个表，每个表配置一种table)
 * 1. plat(0), 2-level (1)
 * 2. 有cache属性， 我们选择 0b110，Inner, Read-allocate, Write-allocate, Write-through
 *    外部cache属性：我们选择 1，没有外部cache
 * 3. 共享属性：0b01,设置成内部共享
 * 4. 表类型设置：1:device table; 2:vPES; 4: collections
 * 5. 要读取entry_size来决定每个entry有多大,这个是只读的
 * 6. 设置表的物理地址（低12位为0，但要求和Page size对齐）
 * 7. 定义Page size
 * 8. 本表的大小 n; 表示使用了多少个物理pages，比如n=0,表示 (n+1)个page
 * 9. 使用有效
 * ------------------------------------------------------------------
 * ITS command的环形表
 * 1. 内部和外部的cache属性
 * 2. 共享属性
 * 3. 物理地址（64K对齐，大小要求4K的倍数 [15:12]要求为0b0000）
 * 4. size: 真实的bytes大小为(size+1) *4K
 * --------------------------------------------
 * ITS 控制寄存器
 * 1.可以读取ITS操作是否都已经完成，用于power management
 * 2. UMSIirq。用于控制Unmaped MSI中断是能，这里我们不涉及这种中断，先用0禁用掉
 * 3. ITS_Number:针对GICv4的，用于设置ITS的实例个数。
 * 4. ITS使能
 * 可以判断ITS命令是否完成
 * ---------------------------------------
 * 读写指针的控制，当出现错误后，可以通过读写retry bit位来实现该command的复位
 * 从名字上来看，应该只影响错误的这个command，让其恢复到未写之前的位置
 * 可以通过读GICI_IIDR来识别ITS的base地址是否设置正确
 * 
 * PMG, PARTID是什么用的？
 * ----------------------------------------------------------------
 * ITS SGI 产生一个虚拟的SGI（V4）
 * ITS状态寄存器 GITS_STATUSR，可查看配置错误原因
*/
void its_set_lpi_config_table_addr(uint32_t rd, uint64_t addr, uint64_t attributes, uint32_t INTIDbits)
{
    // GICI_BASERD[rd] = addr;
    uint64_t tmp = 0;
    tmp = (uint64_t)addr;
    tmp |= (uint64_t)attributes << 32;
    tmp |= (uint64_t)INTIDbits << 48;
 //   gic_its->GITS_LPICFG[rd] = tmp;


}

/**
 * LPI 支持两种类型的中断其中之一：
 * 1. 使用ITS转换的LPI （目前使用这种，物理的ITS至少要有一个用于接收MSI,然后将MSI转化成LPI）
 * 2. 直接使用LPI（这种情况不需要ITS的支持，直接将LPI中断发送到RD）
 * 3. LPI主要通过配置GICR（RD）来实现（支持最小8K的中断个数）
 *    因为中断源多，其相关的配置由table来配合实现，主要是以下两种table（需要在非安全的地址空间来定义）
 *    3.1 实现每个中断的配置（优先级和enable） (全局的 table)
 *        在GICD.DS=0时，LPI总是非安全group1的中断
 *                 =1时，LPI是group1中断
 *        全局配置修改后需要写GICR_INV或GICR_IINVALL命令来实现更新。
 *    3.2 实现每个中断的Pending （每个GICR独有的）
 *    3.3 LPI还有一个enbale位，用于控制RD到PE的LPI通路
 * 
*/
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
    uint32_t size = 1;

    table = alloc_page_table_aligned(PAGE_ALIGN_64K, size);
    memset(table, 0, size * 4096);
    // Command queue table ...
    config_value = (1ull << 63) | (0b110ull << 59) | (0b110ull << 53) \
         |(0b10 << 10) |  (size -1) | (uint64_t)table;
    write64_separately(&gic_its->GITS_CBASER, config_value);

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