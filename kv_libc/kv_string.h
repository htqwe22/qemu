/**********************************************************************************
 * FILE : kv_string.h
 * Description:
 * Author: Kevin He
 * Created On: 2021-01-13 , At 15:18:29
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_STRING_H
#define KV_STRING_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct bytes_t
{
	uint8_t *data;
	uint32_t len;
}bytes_t;


#define is_bcd(b) ((b) >= 0 && (b) <= 0)

static inline uint8_t hexchar_to_bin(char hex)
{
	if (hex >= '0' && hex <= '9')
		return hex-'0';
#if 1	
	hex |= 0x20;		//lowercase to lowercase
#else	
	if (hex >= 'A' && hex <= 'F')
		return hex-'A'+10;
#endif
	if (hex >= 'a' && hex <= 'f')
		return hex-'a'+10;	
	return 0;
}


bytes_t new_bytes(const uint8_t *data, uint32_t length);

void delete_bytes(bytes_t bytes);

int host_is_litte_endian(void);


int hex_string_to_bin(unsigned char *binbuf, int binbuff_size, const char *hexstr, int hexstr_len);

/**/
int bin_to_hex_string(char *hexstr, int hexstr_size, const unsigned char *binbuf, int binbuff_len);


void *memmove2(void *dest, const void *src, size_t n);


char *strstr2(const char *origin, int origin_len, const char *needle);


void swap_byte_arr(uint8_t arr[], int arr_size);

void swap_short_arr(uint16_t arr[], int arr_size);

void swap_int_arr(uint32_t arr[], int arr_size);

void swap_u64_arr(uint64_t arr[], int arr_size);

char *itoa_u64(uint64_t num, char buff[20]);


uint32_t bytes_to_num(const uint8_t bytes[], int len, uint8_t littleendian);

uint32_t bytes_to_uint32(const uint8_t bytes[], uint8_t littleendian);

uint16_t bytes_to_uint16(const	   uint8_t bytes[2], uint8_t littleendian);

void uint16_to_bytes(uint16_t num, uint8_t bytes[2], uint8_t littleendian);

void uint32_to_bytes(uint32_t num, uint8_t bytes[4], uint8_t littleendian);


#ifdef __cplusplus
}
#endif

#endif //kv_string.h
