/**********************************************************************************
 * FILE : exception_common.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-02 , At 21:39:52
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "exception_common.h"
#include <arch_helpers.h>
#include "log.h"


extern const struct exception_entry el3_exception_table[4];
extern const struct exception_entry el1_s_exception_table[4];
extern const struct exception_entry el1_ns_exception_table[4];

//extern uint64_t el3_exceptions[];
static const char *exception_type[] = {
    "Sync",
    "IRQ",
    "FIQ",
    "SError",
};
static const char *exception_desc[] = {
    "CUR_EL_SP0",
    "CUR_EL_SPX",
    "LOW_EL_A64",
    "LOW_EL_A32",
};

static const char *esr_name[0x40] = {
    [1] = "WFx",
    [3] = "CP15_32", // trapped MCR/MRC
    [4] = "CP15_64", // trapped MCRR/MRRC
    [5] = "CP14_MR", // mcr/MRC
    [6] = "CP14_LS", // LDC/STC
    [7] = "FP_ASIMD", // sve, simd, or float.
    [0x0e] = "ILL", // illegal execution state
    [0x11] = "SVC32", //aarch32
    [0x12] = "HVC32", // aarch64 HVC call
    [0x15] = "SVC64", // aarch64 SVC call
    [0x18] = "SYS64", // aarch64 msc/mrs except
    [0x19] = "SVE", // access SVE
    [0x20] = "IABT_LOW", // low level excetpion
    [0x21] = "IABT_CUR", // current level exception
    [0x22] = "PC_ALIGN", // pc alignment fault
    [0x24] = "DABT_LOW", // low level data abortion
    [0x25] = "DABT_CUR", // current level data abortion
    [0x26] = "SP_ALIGN", // sp alignment fault
    [0x28] = "FP_EXE32", //aarch32 floating point exception
    [0x2c] = "FP_EXE64", //aarch64 floating point exception
    [0x2f] = "SERROR", // system error
    [0x30] = "BRK64", // lower level breakpoint
    [0x31] = "BRK64_CUR", // current level breakpoint
    [0x32] = "SOFTSTP_LOW", // lowlevel software step exception
    [0x33] = "SOFTSTP_CUR", // current level software step exception
    [0x34] = "WATCHPT_LOW", // low level watchpoint exception
    [0x35] = "WATCHPT_CUR", // current level watchpoint exception
    [0x36] = "BKPT32", // aarch32 BKPT instruction exception
    [0x37] = "BKPT64", // aarch64 BKPT instruction exception
};

#define debug_xn(Xn) \
    do { \
        uint64_t reg; \
        __asm__ volatile ("mov %0, " #Xn  : "=r" (reg)); \
        kv_debug_raw(LOG_LVL_INFO, #Xn ": 0x%lx\n", reg); \
    } while (0)

void excep_info(uint64_t offset, int el)
{
    uint64_t elr ; 
    uint64_t spsr; 
    uint64_t esr ; 
    uint64_t far ; 
    switch(el) {
    case 3:
        elr = read_elr_el3();
        spsr = read_spsr_el3();
        esr = read_esr_el3();
        far = read_far_el3(); 
    break;

    case 2:
        elr = read_elr_el2();
        spsr = read_spsr_el2();
        esr = read_esr_el2();
        far = read_far_el2(); 
    break;

    case 1:
        elr = read_elr_el1();
        spsr = read_spsr_el1();
        esr = read_esr_el1();
        far = read_far_el1(); 
    break;
    default :
        return;
        kv_debug_raw(LOG_LVL_INFO, COLOR_RED);


    }


    union reg_spsr *spsr_info = (union reg_spsr *)&spsr;
    union reg_esr *esr_info = (union reg_esr *)&esr;

    kv_debug_raw(LOG_LVL_INFO, COLOR_PURPLE);
    kv_debug_raw(LOG_LVL_INFO, "offset %lx\n", offset);
    kv_debug_raw(LOG_LVL_INFO, "ELR_el3=%lx\n", elr);
    kv_debug_raw(LOG_LVL_INFO, "esr_el3=%lx\n", esr);
    kv_debug_raw(LOG_LVL_INFO, "\tEC=%#x(%s)\n", esr_info->EC, esr_name[esr_info->EC]);
    kv_debug_raw(LOG_LVL_INFO, "\tISS=%#x\n", esr_info->ISS);
    kv_debug_raw(LOG_LVL_INFO, "far_el3 %lx\n", far);
    spsr_info = (union reg_spsr *)&spsr;
    kv_debug_raw(LOG_LVL_INFO, "From EL_%d.%s (%s)\n",  spsr_info->M >> 2, exception_type[(offset & 511) >> 7], exception_desc[offset >> 9]);

    uint64_t trace_sp = spsr_info->M >> 2;
    if (spsr_info->M & 1) {
        switch (trace_sp)
        {
        case 0:
            __asm__ volatile ("mrs %0, sp_el0\n"  : "=r" (trace_sp));
        break;
        case 1:
            __asm__ volatile ("mrs %0, sp_el1\n"  : "=r" (trace_sp));
        break;
        case 2:
            __asm__ volatile ("mrs %0, sp_el2\n"  : "=r" (trace_sp));
        break;
        case 3:
        __asm__ volatile ("mov %0, sp\n"  : "=r" (trace_sp));
        break;
        default:
            break;
        }
    } else {
        __asm__ volatile ("mrs %0, sp_el0\n"  : "=r" (trace_sp));
    }
    debug_xn(x0);
    debug_xn(x1);
    debug_xn(x2);
    debug_xn(x3);
    debug_xn(x4);
    debug_xn(x5);
    debug_xn(x6);
    debug_xn(x7);
    debug_xn(x8);
    debug_xn(x9);
    debug_xn(x10);
    debug_xn(x11);
    debug_xn(x12);
    debug_xn(x13);
    debug_xn(x14);
    debug_xn(x15);
    debug_xn(x16);
    debug_xn(x17);
    debug_xn(x18);
    debug_xn(x19);
    debug_xn(x20);
    debug_xn(x21);
    debug_xn(x22);
    debug_xn(x23);
    debug_xn(x24);
    debug_xn(x25);
    debug_xn(x26);
    debug_xn(x27);
    debug_xn(x28);
    debug_xn(x29);
    debug_xn(x30);

    kv_debug_raw(LOG_LVL_INFO, COLOR_NONE);
}



void set_exception_table_el3(void)
{
    write_vbar_el3((u_register_t)el3_exception_table);
    write_daifclr(0xf); 
}

void set_exception_table_el1_s(void)
{
    write_vbar_el1((u_register_t)el1_s_exception_table);
    write_daifclr(0xf); 
}

// void set_exception_table_el1_ns(void)
// {
//     write_vbar_el1((u_register_t)set_exception_table_el1_ns);
//     write_daifclr(0xf); 
// }

void test1(void)
{
    uint64_t FP;
    LOG_DEBUG("this is test 1\n");
    asm volatile ("mov %0, fp\n" : "=r" (FP));
    debug_callstack((void *)FP);
    // using ./sh addr CALL1 CALL2 CALL3 ...
}



void test_exception(void)
{  
    LOG_DEBUG("EL3 exception table at %p, size of table %lu\n", el3_exception_table, sizeof(el3_exception_table)); 
    uint8_t data[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    set_exception_table_el3();
    uint64_t *ptr = (uint64_t *)(data + 2);
    *ptr = 0x123456789abcdef0;
    
 //   kv_debug_data("data", data, 12);
}

