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

 * 
 */


void app_interrupt_init(void)
{
    uint32_t redis_id;
    LOG_DEBUG("----------------\n");
    uint32_t affi = getAffinity();
    LOG_DEBUG("cur affinity: %d.%d.%d.%d\n", affi>>24, (affi>>16)&0xff, (affi>>8)&0xff, affi&0xff);
    setGICAddr((void*)GICD_BASE, (void*)GICR_BASE);
    // 通过读这个寄存器知道GICD的地址已经对的（0x43b）
    LOG_DEBUG("GICD_IIDR: %x\n", gic_dist->GICD_IIDR);
    //通过这里可以看出当前在EL3下实现了4bit的优先级。
    LOG_DEBUG("fixed priority bites: %d\n", get_fixed_Priority_bits());
    enableGIC();
    // 根据亲和性匹配到 redistributor ID
    redis_id = getRedistID(getAffinity());
    // wakeup本PE对应的redistributor ...
    wakeUpRedist(redis_id);

    // 配置CPU interface（好像很复杂，有GICC和ICC的寄存器）
    setPriorityMask(0xFF); // 优先级屏蔽寄存器,这里设置为最低，表示不屏蔽

    enableGroup0Ints();
    enableGroup1Ints();
    enableNSGroup1Ints();  // This call only works as example runs at EL3

}