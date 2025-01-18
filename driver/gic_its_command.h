/**********************************************************************************
 * FILE : gic_its_command.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-18 , At 19:23:52
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_GIC_ITS_COMMAND_H
#define KV_GIC_ITS_COMMAND_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


void its_create_mapd(uint64_t cmd[4], uint32_t dev_id, uint8_t ev_id_bits, uint32_t itt_entry_size);

void its_create_mapti(uint64_t cmd[4], uint32_t dev_id, uint32_t ev_id, uint32_t INITID, uint32_t collect_id);

void its_create_mapc(uint64_t cmd[4], uint32_t collect_id, uint64_t rd_base_or_processNum);

void its_create_sync(uint64_t cmd[4], uint64_t rd_base_or_processNum);

void its_create_inv(uint64_t cmd[4], uint32_t dev_id, uint32_t ev_id);

void its_create_invall(uint64_t cmd[4], uint32_t collection_id);


/**
 *  This is used to create an its interrupt by software.
*/
void its_create_int(uint64_t cmd[4], uint32_t dev_id, uint32_t ev_id);



void its_send_command(uint64_t cmd[4], uint64_t its_base);


#ifdef __cplusplus
}
#endif

#endif //gic_its_command.h
