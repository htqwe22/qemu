/**********************************************************************************
 * FILE : el_switch_test.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-06 , At 19:37:11
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "el_switch_test.h"
#include <arch_helpers.h>
#include <log.h>

extern int test_print(const char *ptr, ...);

static int aaa_pl(int a, int b)
{
    int len = 0;
	char buffer[PRINT_BUFFER_SIZE];
    return a + b;
}

void el2_entry(void)
{
    uint64_t el = get_current_el();
    el = read_daif();
    aaa_pl(1, 2);
    set_debug_level(3);
    const char *ptr = "kevin he, cur_el";
    test_print(ptr);
//     while(*ptr) {
//         pl011_putc(*ptr++);
//     }
//    kv_printf("\n");
//    pl011_putc('K');
//    for (;;);
}




// void switch_to_el2(void)
// {
//  //   asm volatile("msr sp_el2, %0" : : "r"(xxx)); Has setup before
//     uint64_t el2_entry_piont = (uint64_t)&el2_entry;
//     __asm__ volatile ("msr daifset, #15");

    
//     write_elr_el3(el2_entry_piont); // set enter point ...
//     asm volatile("eret\n"
//                  "isb");
// }