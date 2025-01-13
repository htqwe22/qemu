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
#include <asm_func.h>
#include <plat_def.h>
#include <arch_helpers.h>
#include <log.h>

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

