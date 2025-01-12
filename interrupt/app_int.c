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


void gic_global_init(void)
{
    // 找到GICD GICR
    setGICAddr((void*)GICD_BASE, (void*)GICR_BASE);
    // 通过读这个寄存器知道GICD的地址已经对的（0x43b）
    LOG_DEBUG("GICD_IIDR: %x\n", gic_dist->GICD_IIDR);
    // Enable ARE & Group NO 1-of-N
    enableGIC();
    debug_gicd();
}


void gic_current_pe_init(void)
{
    uint32_t redis_id;
//    LOG_DEBUG("cur affinity: %d.%d.%d.%d\n", affi>>24, (affi>>16)&0xff, (affi>>8)&0xff, affi&0xff);
    //通过这里可以看出当前在EL3下实现了4bit的优先级。
    LOG_DEBUG("priority support bits: %d\n", get_fixed_Priority_bits());
    getGICRTyper();

  //  getGICRTyper();
    // 根据亲和性匹配到 redistributor ID
    redis_id = getRedistID(getAffinity());
    // wakeup本PE对应的redistributor ...
    wakeUpRedist(redis_id);
    
    // 配置CPU interface（好像很复杂，有GICC和ICC的寄存器）
    // 看文档，在复位时设定SRE=0，只能使用GICC的寄存器来配置CPU interface ...
    // 但仿真没有遇到问题，先读一下这个寄存器的值吧
    uint32_t ser = getICC_SRE_EL3();
    LOG_DEBUG("ICC_SRE_EL3: %x, %x, %x\n", ser, getICC_SRE_EL2(), getICC_SRE_EL1());
    //here we can use ICH_* & ICC_* to access CPU Interface registers 
    setPriorityMask(0xFF); // 优先级屏蔽寄存器,这里设置为最低，表示不屏蔽

    enableGroup0Ints();
    enableGroup1Ints();
    // ICC_IGRPEN1_EL3 can also be used to enable interrupts for NS & S group 1
    // This call only works as example runs at EL3
    enableNSGroup1Ints(); 

    uint32_t icc_ctrl = 0; 
    icc_ctrl |= (0x7 << 2); // EOI = 0 for group0, S-1,NS-1


    setICC_CTLR_EL3(icc_ctrl);


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


int gic_configure_spi(int INTID, int priority, int target_cpu, int enable)
{
#if 0    
    uint32_t affinity = getAffinity();
    uint32_t rd = getRedistID(getAffinity());
    setIntPriority(1056, rd, 0);
    setIntGroup(1056, rd, GICV3_GROUP0);
    enableInt(1056, rd);

    // GICv3.1 Extended SPI range (INTID 4096)
    setIntPriority(4096, 0, 0);
    setIntGroup(4096, 0, GICV3_GROUP0);
    setIntRoute(4096, GICV3_ROUTE_MODE_COORDINATE, affinity);
    setIntType(4096, 0, GICV3_CONFIG_EDGE);
    enableInt(4096, 0);


    //
    // Trigger PPI in GICv3.1 extended range
    //

    // Setting the interrupt as pending manually, as the
    // Base Platform model does not have a peripheral
    // connected within this range
    setIntPending(1056, rd);
    setIntPending(4096, 0);
#endif
    return 0;
}


