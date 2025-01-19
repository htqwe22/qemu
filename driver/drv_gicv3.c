/**********************************************************************************
 * FILE : drv_gicv3.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-17 , At 11:10:11
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "drv_gicv3.h"
#include <gicv3_registers.h>
#include <asm_func.h>
#include <plat_def.h>
#include <arch_helpers.h>
#include <log.h>
#include <mem_map.h>
#include "gic_its_command.h"


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


static int local_register_irq_handler(uint32_t intid, irq_handler_t handler, void *arg);

int local_unregister_irq_handler(uint32_t intid) ;

static struct GICv3_dist_if*       gic_dist;
static struct GICv3_rdist_if*      gic_rdist;
static uint32_t gic_max_rd = 0;



uint32_t getRedistID(uint32_t affinity)
{
    uint32_t index = 0;
    do {
        if (gic_rdist[index].lpis.GICR_TYPER[1] == affinity)
            return index;
        index++;
    }while(index < gic_max_rd);
    LOG_ERROR("Can not find RD by affinity %#x\n", affinity);
    return 0xFFFFFFFF; // return -1 to signal not RD found
}

uint32_t wakeUpRedist(uint32_t rd)
{
  uint32_t tmp;

  // Tell the Redistributor to wake-up by clearing ProcessorSleep bit
  tmp = gic_rdist[rd].lpis.GICR_WAKER;
  tmp = tmp & ~0x2;
  gic_rdist[rd].lpis.GICR_WAKER = tmp;

  do {
    tmp = gic_rdist[rd].lpis.GICR_WAKER;
  } while((tmp & 0x4) != 0);

  return 0;
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


uint64_t debug_its_typer(uint64_t its_base)
{
    struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;
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

void write64_separately(volatile uint32_t *addr, uint64_t val)
{
    *addr++ = val & 0xffffffff;
    *addr = (val >> 32)& 0xffffffff;
}

uint64_t read64_separately(volatile uint32_t *addr)
{
    return addr[0] | (uint64_t)addr[1] << 32;
}


int gicv3_wakeup_pe(uint32_t affinity)
{
    //1. Wakeup PE
    uint32_t rd = getRedistID(affinity);
    wakeUpRedist(rd);
    return 0;
}


static void set_sgi_ppi_priorty(uint32_t rd, uint32_t INTID, uint8_t priority)
{
    uint32_t idx, offset, tmp;
    if (INTID > 31)
        return;
    idx = INTID >> 2; // /4
    offset = (INTID & 3) << 3; // * 8

    tmp = gic_rdist[rd].sgis.GICR_IPRIORITYR[idx];
    tmp &= ~(0xff << offset);
    tmp |= (priority << offset);
    gic_rdist[rd].sgis.GICR_IPRIORITYR[idx] = tmp;
}

static void set_sgi_ppi_trigger(uint32_t rd, uint32_t INTID, irq_trigger_t trigger)
{
    uint32_t tmp, offset, idx;
    if (INTID > 31)
        return;
    idx = INTID >> 4; // /16
    offset = (INTID & 16) << 1; // (MOD 16) * 2

    tmp = gic_rdist[rd].sgis.GICR_ICFGR[idx];
    tmp &= ~(0x3 << offset);
    tmp |= trigger;
    gic_rdist[rd].sgis.GICR_ICFGR[idx] = tmp;
}


static void set_sgi_ppi_group(uint32_t rd, uint32_t INTID, int_group_t group)
{
    uint32_t offset;
    uint32_t tmp;
    if (INTID > 31)
        return;

    offset = (INTID & 31);
    tmp = gic_rdist[rd].sgis.GICR_IGRPMODR[0];
    tmp &= ~(1 << offset);
    tmp |= (group >> 1) << offset;
    gic_rdist[rd].sgis.GICR_IGRPMODR[0] = tmp;

    tmp = gic_rdist[rd].sgis.GICR_IGROUPR[0];
    tmp &= ~(1 << offset);
    tmp |= (group & 1) << offset;
    gic_rdist[rd].sgis.GICR_IGROUPR[0] = tmp;
}

void gicv3_sgi_ppi_enable(uint32_t target_affi, uint32_t INTID, bool enable)
{
    uint32_t offset, tmp;
    if (INTID > 31)
        return;
    uint32_t rd = getRedistID(target_affi);
    offset = (INTID & 31);
    tmp = gic_rdist[rd].sgis.GICR_ISENABLER[0];
    tmp &= ~(1 << offset);
    if (enable)
        tmp |= 1 << offset;

    gic_rdist[rd].sgis.GICR_ISENABLER[0] = tmp;   
}


void gicv3_spi_enable(uint32_t INTID, bool en)
{
    uint32_t tmp = gic_dist->GICD_ISENABLER[INTID >> 5];
    tmp &= ~(1 << (INTID & 0x1f));
    if (en)
        tmp |= 1 << (INTID & 0x1f);
    gic_dist->GICD_ISENABLER[INTID >> 5] = tmp;
}

/**
 *  To change the configuration of an interrupt, software writes to the LPI Configuration tables and then issues the INV
    or INVALL command. In implementations that do not include an ITS, software writes to GICR_INVALLR or
    GICR_INVLPIR.
*/
void gicv3_lpi_set_priority(uint32_t INTID, uint32_t affinity, uint8_t priority)
{
    uint32_t rd = getRedistID(affinity);
    uint8_t config;
    uint8_t *prop_table = (uint8_t*)(gic_rdist[rd].lpis.GICR_PROPBASER & 0x0000FFFFFFFFF000);

  // Mask off unused bits of the priority and enable
    config = (priority & 0x7c) | 0b10 | 1;
    // Note: bit 1 is RES1
    prop_table[INTID - 8192] = config;
    dsb();
    // set GICR_INVPIR from cache. The Next statement may causes bugs.
    // gic_rdist[rd].lpis.GICR_INVLPIR = INTID;
}


/***************************************************************************/

/**
 *  @brief 初始化GICv3
 *  @param irq_in_el3: 是否在EL3下接受IRQ
 *  @param fiq_in_el3: 是否在EL3下接受FIQ
 *  @param external_abort_in_el3: 是否在EL3下接受外部中止
 *  @return 0 成功，-1 失败
 *  @note should access at EL3
*/
int gicv3_global_init(bool irq_in_el3, bool fiq_in_el3, bool external_abort_in_el3)
{
    gic_dist = (struct GICv3_dist_if*)GICD_BASE;
    gic_rdist = (struct GICv3_rdist_if*)GICR_BASE;
    gic_max_rd = 0;
    while((gic_rdist[gic_max_rd].lpis.GICR_TYPER[0] & (1<<4)) == 0) // Keep incrementing until GICR_TYPER.Last reports no more RDs in block
    {
        gic_max_rd++;
    }
//    LOG_DEBUG("get max rd num %d\n", gic_max_rd);
    /**
     *  GICD_CTLR AES = 1
     *  1. ARE_S/NS = 1 to use All GICv3
     *  2. DS = 0, use 2 security states
     *  3. Enable Group0/1S/1NS functions
    */
    // First set the ARE bits
    gic_dist->GICD_CTLR = (1 << 5) | (1 << 4);
    // The split here is because the register layout is different once ARE==1
    // Now set the rest of the options
    gic_dist->GICD_CTLR = 7 | (1 << 5) | (1 << 4);

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
    return 0;
}

int gicv3_current_cpu_interface_init(void)
{
//    uint32_t rd = getRedistID(affinity);
    //1.  Enable System register access(SRE)
    write_icc_sre_el3(0x9);  // 1001, enable lower ELs to access SRE_EL3 & SER_EL3 = 1;
//    write_icc_sre_el2(0x9) & bpr1; 
    uint64_t tmp = read_scr_el3();
    tmp |= SCR_NS_BIT;
    write_scr_el3(tmp);
    dsbsy();
    isb();
    write_icc_sre_el1(0x1); // for ns el1
    dsbsy();
    isb();
    write_icc_bpr1_el1(1);


    tmp &= ~SCR_NS_BIT;
    write_scr_el3(tmp);
    dsbsy();
    isb();
    write_icc_sre_el1(0x1); // for s el1
    dsbsy();
    isb();
    write_icc_bpr1_el1(1);

    //2. Set priority mask and binary point registers
    /**
     *  Binary point value Group priority field Subpriority field Field with binary point
     *    0                 [7:1]                 [0]                 ggggggg.s
     *    1                 [7:2]               [1:0]                 gggggg.ss
     */
    //低两位是子优先级，高6位是组优先级
    write_icc_pmr_el1(0xff); // set to 0xff to enable irq.
    //3. BPR0 as 2
    write_icc_bpr0_el1(0b10);
    
    //4. Set EOI mode
    // EOI = 0, others = 0
    tmp = (1 << 6) | (0b0 << 5) | (0b000 << 2);
    write_icc_ctlr_el3(tmp);
    tmp =  (1 << 6) | (0b0 << 1) | (0b00 << 0);
    write_icc_ctlr_el1(tmp);

    //5. Enable group control 
    write_icc_igrpen1_el3(3);// Group 1 S & NS
    isb();
    write_icc_igrpen0_el1(1);
    return 0;
}


/**
 *  我们要求支持的LPI的个数不能导致最大中断id的bits不能超过GICD.typer的idbits)
 *  @param affinity: 中断的亲和性
*/
int gicv3_lpi_enable(uint32_t affinity, uint32_t max_lpi_num)
{
    // for pending table
    uint64_t tmp;
    uint32_t rd = getRedistID(affinity);

    uint32_t max_id = 8192 + max_lpi_num;
    uint32_t num;

    // 1. set GICR_PENDBASER
    for (num = 1; max_id > num * (4096 << 3); num++);
    void *p_table = alloc_page_table_aligned(PAGE_ALIGN_64K, num);
    // Clear pending table
 //   LOG_DEBUG("p_table at %p\n", p_table);
    memset_64(p_table, 0, num * 4096);

    // 0b110 cacheable, write-allocate, read-allocate, write-through
    // 0b01 at bit10; Inner Shareable
    tmp = (0b110ull << 7) | (0b01ull << 10) | (uint64_t)p_table; // & 0x0000FFFFFFFF0000
    gic_rdist[rd].lpis.GICR_PENDBASER = tmp;
//    LOG_DEBUG("LPI Pending base %lx\n", gic_rdist[rd].lpis.GICR_PENDBASER);
    // set GICR_PROPBASER for eatch rd. and use the same prop_table.
    for (num = 1; max_lpi_num > 4096 * num; num++);
    static void *sh_prop_table = NULL;
    if (sh_prop_table == NULL) {
        sh_prop_table = alloc_page_table_aligned(PAGE_ALIGN_4K, num);
        memset(sh_prop_table, 0, num);
    }
    // 0b110 cacheable, write-allocate, read-allocate, write-through
    // 0b01 at bit10; Inner Shareable
    // Outer & Inner Shareable, inner share.
    tmp = (0b110ull << 56) | (0b110ull << 7) | (0b10ull << 10) | (uint64_t)sh_prop_table;
    for (num = 14; (1ull << num) < max_id; num++);
    gic_rdist[rd].lpis.GICR_PROPBASER = tmp | (num - 1);
    // LOG_DEBUG("table at %p\n", sh_prop_table);
    // LOG_DEBUG("LPI prob base %lx\n", gic_rdist[rd].lpis.GICR_PROPBASER);
    
    // SET 1 of N for eatch group and enable
    num = (0b111u << 24) | 1;
    gic_rdist[rd].lpis.GICR_CTLR = num;
    return 0;
}

int gicv3_its_init(uint64_t its_base)
{
    uint64_t its_typer;
    uint64_t config_value;
    uint32_t tmp;
    uint32_t idx, dev_idx = 0xff, collection_idx = 0xff;
    uint32_t type;
    uint32_t entry_size, dev_entry_size, collection_entry_size;
    uint32_t pg_value = 0; //default 4KB

    // 1. set its base address
    struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;
    its_typer = gic_its->GITS_TYPER;
    LOG_DEBUG("its_typer %#x\n", its_typer);
    // 2. CTLR setting ...
//    tmp = (gic_its->GITS_CTLR >> 4) & 0x0f;
//    LOG_DEBUG("its num %d\n", tmp);
    gic_its->GITS_CTLR = 0; // nothing to be configured.

    //3 configure device table & connection type
    for (idx = 0; idx < 8; idx++) {
        type = (uint32_t)(0x7  & (gic_its->GITS_BASER[idx] >> 56));
        entry_size = (uint32_t)(0x1F & (gic_its->GITS_BASER[idx] >> 48));
//        LOG_DEBUG("idx %d, type %d, pg_size %d, entry_size %d\n", idx, type, (gic_its->GITS_BASER[idx] >> 8) & 3, entry_size);
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
    uint32_t page_size;
#ifdef FIXED_PAGE_SIZE
    pg_value = (gic_its->GITS_BASER[dev_idx] >> 8) & 3;
    switch(pg_value) {
        case 0:
            page_size = 4*KB;
            break;
        case 1:
            page_size = 16*KB;
            break;
        case 2:
        case 3:
            page_size = 64*KB;
            break;
    }
#else
    page_size = ITS_PAGE_SIZE;
#endif
    // 3.1 configure device table
    uint32_t id_bits;
    id_bits = ((its_typer >> 13) & 0x1f) + 1; // devid bits
    if (id_bits < USED_DEV_ID_BITS) {
        LOG_ERROR("defined ID bits > supported id bits(%u)\n", id_bits);
        return -2;
    }
    id_bits = USED_DEV_ID_BITS;
//    LOG_DEBUG("Device ID bits is %d, page size %d, entry size %d\n", id_bits, page_size, dev_entry_size);
    tmp = (1ul << id_bits) * dev_entry_size; // all device table size in bytes;
    idx = (tmp + page_size - 1) / page_size; // pages number.
    table = alloc_page_table_aligned(page_size >> 12, idx * page_size/4096); 
 //   LOG_DEBUG("DEV table at %p-%x(size %u)\n", table, (uint64_t)table + idx * page_size, idx * page_size);
    memset_64(table, 0, idx * page_size); //clean the table
    // Command queue table ...
    config_value = (1ull << 63) | (0b110ull << 59) | (0b110ull << 53) \
         |(0b10 << 10) |  (idx -1) | (uint64_t)table;
    write64_separately((volatile uint32_t *)&gic_its->GITS_BASER[dev_idx], config_value);
    dsb();
    isb();
 //   LOG_DEBUG("DEV configure %lx\n", gic_its->GITS_BASER[dev_idx]);
    
    // 3.2 Collection table ..
#ifdef FIXED_PAGE_SIZE
    page_size = (gic_its->GITS_BASER[collection_idx] >> 8) & 3;
    switch(page_size) {
        case 0:
            page_size = 4*KB;
            break;
        case 1:
            page_size = 16*KB;
            break;
        case 2:
        case 3:
            page_size = 64*KB;
            break;
    }
#else
    page_size = ITS_PAGE_SIZE;
#endif
    if (its_typer & (1ull << 36)) {
        id_bits = ((its_typer >> 32) & 0xf) + 1; // devid bits
    }else{
        id_bits = 16;
    }
    if (id_bits < USED_DEV_ID_BITS) {
        LOG_ERROR("defined ID bits > supported id bits(%u)\n", id_bits);
        return -2;
    }
    id_bits = USED_DEV_ID_BITS;

 //   LOG_DEBUG("Collection ID bits is %d, page size %d, entry size %d\n", id_bits, page_size, collection_entry_size);
    tmp = (1ul << id_bits) * collection_entry_size; // all device table size in bytes;
    idx = (tmp + page_size - 1) / page_size; // pages number.
    table = alloc_page_table_aligned(page_size >> 12, idx * page_size/4096); 
 //   LOG_DEBUG("Collection table at %p-%x (size %u)\n", table, (uint64_t)table + idx * page_size, idx * page_size);
    memset_64(table, 0, idx * page_size); //clean the table
    // Command collection table ...
    config_value = (1ull << 63) | (0b110ull << 59) | (0b110ull << 53) \
         |(0b10 << 10) |  (idx -1) | (uint64_t)table;
    write64_separately((volatile uint32_t *)&gic_its->GITS_BASER[collection_idx], config_value);
    dsb();
//    LOG_DEBUG("Collection configure %lx\n", gic_its->GITS_BASER[collection_idx]);

    //4. configure command queue need 64 K aligned ....
    idx = 1;
    table = alloc_page_table_aligned(PAGE_ALIGN_64K, idx); 
 //   LOG_DEBUG("command table addr %lx- %lx\n", (uint64_t)table, (uint64_t)table + 4096);
    memset_64(table, 0, idx * 4096); //clean the table  
    config_value = (1ull << 63) | (0b110ull << 59) | (0b110ull << 53) \
         |(0b10 << 10) |  (idx -1) | (uint64_t)table;
    write64_separately((volatile uint32_t *)&gic_its->GITS_CBASER, config_value);
    gic_its->GITS_CWRITER = 0;    // This register contains the offset from the start, hence setting to 0
    dsb();
//    LOG_DEBUG("comand configure %lx\n", gic_its->GITS_CBASER);
    //5.enable its
    gic_its->GITS_CTLR |= 1;
    return 0;
}

/**
 * trigger & target_affi is only valid for SPI
*/
int gicv3_spi_register(uint32_t INTID, uint8_t priority, int_group_t group, irq_trigger_t trigger, uint32_t target_affi, irq_handler_t handler, void *priv_arg)
{
    // only SPI support .. (Not include SPI-E)
    if (INTID < 32  || INTID > 1020) {
        LOG_ERROR("SPI ID %d is not supported\n", INTID);
        return -1;
    }

    uint64_t tmp64;
    uint32_t tmp, idx, offset;
    local_register_irq_handler(INTID, handler, priv_arg);
//    uint32_t rd = getRedistID(target_affi);

    //1. set the interrupt priority
    // GICD_IPRIORITYR共255个，每个可控制4个中断，共1020个
    // INITID < 32, 时，对应对SGI/PPI的中断号。前提是要求GICR.process_number < 8
    // 这种情况下IPRIORITYR[7:0]每个都是单独的CPU是bank的
    // 当目标ProccessNumber >=8 时，需要使用 GICR_IPRIORITYR[7:0]来配置优先级
    gic_dist->GICD_IPRIORITYR[INTID] = priority;

    //2. set Trigger Mode // 0b00 LEVEL, 0b10, EDGE （REG：GICD_ICFGR）
    idx = INTID >> 4;  // /16
    offset = (INTID & 0xf) << 1; // (MOD 16) * 2
    tmp = gic_dist->GICD_ICFGR[idx];
    tmp = tmp & ~(0x3 << offset);
    tmp |= (trigger << offset); 
    gic_dist->GICD_ICFGR[idx] = tmp;

    /** 3.
     * set secure state & group; together with Group Modfier & Group secure status
     * GICR_IGROUPR & GICR_IGRPMODR
     */ 
    /** GICD_IGRPMODR   GICD_IGROUPR    Definition
     *   0              0              0: G0S
     *   0              1              1: G1NS
     *   1              0              2: G1S
    */
    idx = INTID >> 5;  // /32
    offset = (INTID & 31); //MOD 32

    tmp = gic_dist->GICD_GRPMODR[idx];
    tmp &= ~(1 << offset);
    tmp |= (group >> 1) << offset;
    gic_dist->GICD_GRPMODR[idx] = tmp;

    tmp = gic_dist->GICD_IGROUPR[idx];
    tmp &= ~(1 << offset);
    tmp |= (group & 1) << offset;
    gic_dist->GICD_IGROUPR[idx] = tmp;

    //4. set router ...
    tmp64 = (uint64_t)(target_affi & 0x00FFFFFF) |
        (((uint64_t)target_affi & 0xFF000000) << 8) |
          (uint64_t)0b0 << 30;
    gic_dist->GICD_ROUTER[INTID] = tmp64;

    //5. enable SPI
    gicv3_spi_enable(INTID, 1);
    return 0;
}


int gicv3_ppi_register(uint32_t INTID, uint8_t priority, int_group_t group, irq_trigger_t trigger, uint32_t target_affi, irq_handler_t handler, void *priv_arg)
{
    uint32_t rd;
    if (INTID < 16  || INTID > 31) {
        LOG_ERROR("PPI ID %d is not supported\n", INTID);
        return -1;
    }
    rd = getRedistID(target_affi);
    local_register_irq_handler(INTID, handler, priv_arg);
    //1. set the interrupt priority
    set_sgi_ppi_priorty(rd, INTID, priority);

    //2. set Trigger Mode // 0b00 LEVEL, 0b10, EDGE 
    set_sgi_ppi_trigger(rd, INTID, trigger);

    /** 
     * 3.set secure state & group;
     */
    set_sgi_ppi_group(rd, INTID, group);

    gicv3_sgi_ppi_enable(target_affi, INTID, 1);
    return 0;
}

int gicv3_sgi_register(uint32_t INTID, uint8_t priority, int_group_t group, uint32_t target_affi, irq_handler_t handler, void *priv_arg)
{
    uint32_t rd = getRedistID(target_affi);
    if (INTID > 15) {
        LOG_ERROR("SGI ID %d is not supported\n", INTID);
        return -1;
    }
    local_register_irq_handler(INTID, handler, priv_arg);
    //1. set the interrupt priority
    set_sgi_ppi_priorty(rd, INTID, priority);

    //2. set Trigger Mode // 0b00 LEVEL, 0b10, EDGE 
    set_sgi_ppi_trigger(rd, INTID, TRIGGER_EDGE);

    /** 
     * 3.set secure state & group;
     */
    set_sgi_ppi_group(rd, INTID, group);
    return 0;
}

int gicv3_its_register(uint64_t its_base, uint32_t INTID, uint8_t priority, uint32_t target_affi,  \
    uint32_t dev_id, uint32_t ev_id, uint32_t collect_id, irq_handler_t handler, void *priv_arg)
{
    if (INTID < 8192) {
        LOG_ERROR("ITS ID %d is not supported\n", INTID);
        return -1;
    }

    uint64_t cmd[4];
    struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;
    uint32_t rd = getRedistID(target_affi);
    local_register_irq_handler(INTID, handler, priv_arg);
    //1. Set priority
    gicv3_lpi_set_priority(INTID, target_affi, priority);
    uint32_t itt_entry_size = ((gic_its->GITS_TYPER >> 4) & 0x1f) + 1;
    LOG_DEBUG("intid %d, itt_entry_size %d\n", INTID, itt_entry_size);
    // MAPD dev_id
    its_create_mapd(cmd, dev_id, USED_EV_ID_BITS, itt_entry_size);
    its_send_command(cmd, its_base);

    // MAPTI
    its_create_mapti(cmd, dev_id, ev_id, INTID, collect_id);
    its_send_command(cmd, its_base);


    // MAPTC
    uint64_t rdBase;
    //The RDbase field consists of bits[51:16] of the address
    // check GITS_TYPER.PTA bit
    if (gic_its->GITS_TYPER & (1ull << 19))
        rdBase = (uint64_t) &gic_rdist[rd] & 0xfffffffff0000ull; 
    else
        rdBase = (gic_rdist[rd].lpis.GICR_TYPER[0] >> 8) & 0xffff;
    its_create_mapc(cmd, collect_id, rdBase);
    its_send_command(cmd, its_base);

    its_create_sync(cmd, rdBase);
    its_send_command(cmd, its_base);
    isb();
    return 0;
}


void gicv3_its_enable(uint64_t its_base, bool en)
{
    struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;
 //   while ((gic_its->GITS_CTLR >> 31) == 0); 
 //   LOG_DEBUG("gic_its->GITS_CTLR = %lx at %p\n", gic_its->GITS_CTLR, &gic_its->GITS_CTLR);
    if (en)
        gic_its->GITS_CTLR |= 1;
    else
        gic_its->GITS_CTLR &= ~1u;
    dsb();
}

void gicv3_debug_its_status(uint64_t its_base)
{
    struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;
    uint32_t status = gic_its->GITS_STATUSR;
    LOG_DEBUG("its status %#x\n", status);
}


void gicv3_set_pending(uint32_t INTID)
{
    uint32_t idx, offset;
    if (INTID < 32) { // sgi, ppi
        gic_rdist->sgis.GICR_ISPENDR[0] |=  (1u << INTID);
    }else if (INTID <= 1020) { //SPI
        idx = INTID >> 5;
        offset = INTID & 0x1f;
        gic_dist->GICD_ISPENDR[idx] |= (1 << offset);
    }else {
        LOG_ERROR("INTID %d is not supported\n", INTID);
    }
}

void gicv3_send_sgi(int_group_t group, uint32_t INTID, uint32_t target_affi, uint16_t list_bitmap)
{
    uint64_t tmp;
    tmp = ((uint64_t)target_affi & 0xff000000) << 24 | ((uint64_t)target_affi & 0xff0000) << 16 |  (target_affi & 0xff00) << 8;
    // Set RS = 0,  TargetList[n] represents aff0 value ((RS * 16) + n).
    // set IRM = 0,  Aff3.Aff2.Aff1.<target list>.
    tmp |= (INTID << 24) & 0xf;
    // target list is 0b101 means the PE whos affinity value is 0 & 2 will recive this interrupt.
    tmp |= list_bitmap;

    if (group == GROUP0) {
        write_icc_sgi0r_el1(tmp);
    }else if (group == GROUP1_S) {
        write_icc_sgi1r(tmp);
    }else if (group == GROUP1_NS) {
        //TODO .. change SCR_EL3 NS = 1
    }else {
        LOG_ERROR("group %d is not supported\n", group);
    }
}

void gicv3_set_lpi_pending(uint64_t its_base, uint32_t target_affi, uint32_t dev_id, uint32_t event_id)
{
    uint64_t cmd[4];
 //   struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;
 //   uint32_t rd = getRedistID(target_affi);
    its_create_int(cmd, dev_id, event_id);
    its_send_command(cmd, its_base);
}


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

int local_unregister_irq_handler(uint32_t intid) 
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
