/**********************************************************************************
 * FILE : gic_its_command.c
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-18 , At 19:23:52
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "gic_its_command.h"
#include <asm_func.h>
#include <gicv3_registers.h>
#include <mem_map.h>
#include <arch_helpers.h>
#include <log.h>

#define COMMAND_SIZE  (32)

struct its_mapd
{
    uint8_t cmd_id;
    uint8_t res0_0[3];
    uint32_t DeviceID;

    uint64_t Size:5;
    uint64_t res0_1:59;

    uint64_t res0_2:8;
    uint64_t ITT_addr :44;
    uint64_t res0_3:11;
    uint64_t V:1;

    uint64_t res0_4;
};


struct its_mapti
{
    uint8_t cmd_id;
    uint8_t res0_0[3];
    uint32_t DeviceID;

    uint32_t EventID;
    uint32_t pINTID;

    uint16_t ICID;
    uint16_t res0_1[3];
    uint64_t res0_2;
};

struct its_mapc
{
    uint8_t cmd_id;
    uint8_t res0_0[7];

    uint64_t res0_1;

    uint64_t ICID :16;
    uint64_t RDbase :35;
    uint64_t res0_2 :12;
    uint64_t V  :1;

    uint64_t res0_3;
};

struct its_sync
{
    uint8_t cmd_id;
    uint8_t res0_0[7];

    uint64_t res0_1;

    uint64_t res0_2 :16;
    uint64_t RDbase :35;
    uint64_t res0_3 :13;

    uint64_t res0_4;
};


struct its_inv
{
    uint8_t cmd_id;
    uint8_t res0_0[3];
    uint32_t DeviceID;

    uint32_t EventID;
    uint32_t res0_1;

    uint64_t res0_2;

    uint64_t res0_3;
};


/**
 * @param event_id_bits: used for checking Event ID bits 
*/
void its_create_mapd(uint64_t cmd[4], uint32_t dev_id, uint8_t ev_id_bits, uint32_t itt_entry_size)
{
    void *itt_table;
    memset_64(cmd, 0, 8 *4);
 //   assert(used_id_size > 0);
    struct its_mapd *cmd_node = (struct its_mapd *)cmd;
    cmd_node->cmd_id = 0x8;
    cmd_node->DeviceID = dev_id;
    cmd_node->Size = ev_id_bits - 1;
    // itt table is 256 bytes aligned, tabe[51:8] is use here. 
    // entry of ITT is implementation defined. 
    // ITT表项的格式是实现定义的。一个典型的表项大小为8字节，这允许以32个中断的倍数为设备分配标识符
    uint32_t all_size = (1ul << ev_id_bits) * itt_entry_size; // all device table size in bytes;
    uint32_t allock_size;
    for (allock_size = 4096; allock_size < all_size; allock_size += 4096);
    // LOG_DEBUG("all_size %d, allock_size %d\n", all_size, allock_size);
    itt_table = alloc_page_table_aligned(PAGE_ALIGN_4K, allock_size >> 12);
    LOG_DEBUG("ITT table addr %lx- %lx\n", (uint64_t)itt_table, (uint64_t)itt_table + allock_size);
    memset_64(itt_table, 0, allock_size);
    LOG_DEBUG("ITT ev bits is %d, entry size %d, alloc size %u\n", ev_id_bits, itt_entry_size, allock_size);
    cmd_node->ITT_addr = ((uint64_t)itt_table >> 8) & 0xfffffffffffull; //[51:8]
    cmd_node->V = 1;
}


void its_create_mapti(uint64_t cmd[4], uint32_t dev_id, uint32_t ev_id, uint32_t INITID, uint32_t collect_id)
{
    memset_64(cmd, 0, 8 *4);
    struct its_mapti *cmd_node = (struct its_mapti *)cmd;

    cmd_node->cmd_id = 0x0a;
    cmd_node->DeviceID = dev_id;
    cmd_node->pINTID = INITID;
    cmd_node->EventID = ev_id;
    cmd_node->ICID = collect_id;
}

void its_create_mapc(uint64_t cmd[4], uint32_t collect_id, uint64_t rd_base_or_processNum)
{
    memset_64(cmd, 0, 8 *4);
    struct its_mapc *cmd_node = (struct its_mapc *)cmd;

    cmd_node->cmd_id = 0x09;
    cmd_node->ICID = collect_id;
    // here we use 
    cmd_node->RDbase = rd_base_or_processNum;
    cmd_node->V = 1;
}

void its_create_sync(uint64_t cmd[4], uint64_t rd_base_or_processNum)
{
    memset_64(cmd, 0, 8 *4);
    struct its_sync *cmd_node = (struct its_sync *)cmd;
    cmd_node->cmd_id = 0x05;
    cmd_node->RDbase = rd_base_or_processNum;
}


void its_create_inv(uint64_t cmd[4], uint32_t dev_id, uint32_t ev_id)
{
    memset_64(cmd, 0, 8 *4);
    struct its_inv *cmd_node = (struct its_inv *)cmd;
    cmd_node->cmd_id = 0x0c;
    cmd_node->DeviceID = dev_id;
    cmd_node->EventID = ev_id;
}

void its_create_invall(uint64_t cmd[4], uint32_t collection_id)
{
    memset_64(cmd, 0, 8 *4); 
    cmd[0] = 0xd;
    cmd[4] = collection_id;
}

void its_create_int(uint64_t cmd[4], uint32_t dev_id, uint32_t ev_id)
{
    memset_64(cmd, 0, 8 *4); 
    uint32_t *ptr = (uint32_t *)cmd;
    ptr[0] = 0x03;
    ptr[1] = dev_id;
    ptr[2] = ev_id;
}


void its_send_command(const uint64_t cmd[4], uint64_t its_base)
{   
    uint32_t queue_size;
    uint64_t new_cwriter, queue_base, queue_offset, queue_read;
    uint64_t* entry; 
    struct GICv3_its_ctlr_if* gic_its = (struct GICv3_its_ctlr_if *)its_base;

    // total size in bytes.
    queue_size = ((gic_its->GITS_CBASER & 0xFF) + 1) * 0x1000;
    queue_base = (gic_its->GITS_CBASER & (uint64_t)0x0000FFFFFFFFF000);
    queue_offset = gic_its->GITS_CWRITER;  // GITS_CWRITER contains the offset
    queue_read   = gic_its->GITS_CREADR;
    LOG_DEBUG("queue size %d, base %lx, offset %lx, read %lx\n", queue_size, queue_base, queue_offset, queue_read);
#if 0
    // Check that the queue is not full
    if (queue_read == 0)
        queue_read = queue_size - COMMAND_SIZE;
    else
        queue_read = queue_read - COMMAND_SIZE;
    // wait for queue is not full
    while (queue_offset == queue_read);
#else
    new_cwriter = queue_offset + COMMAND_SIZE;
    while (new_cwriter == queue_read || new_cwriter ==  queue_read + queue_size);
#endif

    // Get the address of the next (base + write offset)
    entry = (uint64_t*)(queue_base + queue_offset);
    LOG_DEBUG("entry %p, cmd at %p\n", entry, cmd);
    memcpy_64(entry, cmd, COMMAND_SIZE);
    dsb();

    if (new_cwriter >= queue_size) // acture new_cwriter never > queue_size
        new_cwriter = 0;

    gic_its->GITS_CWRITER = new_cwriter; // update write pointer.
    // I DONOT think The next statement is nessary. Kevin.he
    //////// while(gic_its->GITS_CWRITER != gic_its->GITS_CREADR);
    isb();
}