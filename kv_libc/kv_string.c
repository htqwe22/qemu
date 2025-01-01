/**********************************************************************************
 * FILE : kv_string.c
 * Description:
 * Author: Kevin He
 * Created On: 2021-01-13 , At 15:18:29
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "kv_string.h"
#include <stdint.h>

#if 0
#include <string.h>
#else
void *memset(void *s, int c, size_t n)
{
	uint8_t *ptr = (uint8_t *)s;
	if (s && n) {
		while(n--)
			*ptr++ = c;
	}
	return s;
}

 void *memcpy(void *dest, const void *src, size_t n)
 {
	uint8_t *ptr = (uint8_t *)dest;
	const uint8_t *s = (uint8_t *)src;
	if (ptr && s && n) {
		while(n--)
			*ptr++ = *s++;
	}
 }

int memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *ptr = (uint8_t *)s1;
	const uint8_t *s = (uint8_t *)s2;
	int ret;
	if (s1 && s2 && n) {
		do{
			ret = *ptr - *s;
			ptr++, s++;
			n--;
		}while(n && ret == 0);
	}
	return ret;
}

size_t strlen(const char *s)
{
	size_t len = 0;
	if (s) {
		while (*s++)
			len++;
	}
	return len;
}

char *strrchr(const char *s, int c)
{
	int len;
	if (s) {
		len = strlen(s);
		s += len - 1;
		while(len--) {
			if (*s == c)
				return (char *)s;
			s--;
		}
	}
	return NULL;
}


#endif

int host_is_litte_endian(void)
{
	static uint16_t num = 0x0001;
	return *(uint8_t *)&num;
}


char *itoa_u64(uint64_t num, char buff[20])
{
	buff[19] = 0;	
	char *p = buff+18;
	char i;
	while(num) {
		i = num %10;
		num = num/10;
		*p-- = i+'0';
	}
	if (p == buff+18)
		*p = '0';
	else
		p++;
	return p;
}






#define USE_LOWERCASE
#ifdef USE_LOWERCASE
static const char *charlist = "0123456789abcdef";
#else
static const char *charlist = "0123456789ABCDEF";
#endif
#define bin_to_hexchar(b)	charlist[(b)]

#ifndef min 
#define min(a, b)	(a)<(b)?(a):(b)
#endif


/*return real bin length*/
int hex_string_to_bin(unsigned char *binbuf, int binbuff_size, const char *hexstr, int hexstr_len)
{
	if (hexstr == NULL || binbuf == NULL)
		return 0;

	// reuse hexstr_len as result length
	hexstr_len >>= 1;
	hexstr_len = min(hexstr_len, binbuff_size);
	// reuse binbuff_size as index.
	for (binbuff_size = 0; binbuff_size < hexstr_len; binbuff_size++, hexstr += 2) {
		binbuf[binbuff_size] = (hexchar_to_bin(hexstr[0]) << 4) | hexchar_to_bin(hexstr[1]);
	}
	return hexstr_len;
}

/**/
int bin_to_hex_string(char *hexstr, int hexstr_size, const unsigned char *binbuf, int binbuff_len)
{
	if (hexstr == NULL || binbuf == NULL)
		return 0;
	uint8_t bin; // support binbuff is located last to the end of hexStr 
	// get real transferd bin len.
	binbuff_len = min(binbuff_len, hexstr_size>>1);
	
	// reuse hexstr_size as index.
	for (hexstr_size = 0; hexstr_size < binbuff_len; hexstr_size++, hexstr += 2) {
		bin = binbuf[hexstr_size];
		hexstr[0] = bin_to_hexchar(bin >> 4);
		hexstr[1] = bin_to_hexchar(bin & 0xf);
	}
	
	return hexstr_size<<1;
}


void *memmove2(void *dest, const void *src, size_t n)
{
	size_t i;
	if (dest == src || dest == NULL || src == NULL || n == 0)
		return dest;
	if ((const char *)src < (char *)dest && ((char *)dest - (const char*)src < (signed)n)) {
		for (; n ; n--)
			*((char*)dest + n - 1) = *((const char*)src + n - 1);
	}else{
		for (i = 0; i < n; i++)
			*((char*)dest + i) = *((const char*)src + i);
	}
	return dest;
}

char *strstr2(const char *origin, int origin_len, const char *needle)
{
	int i;
	int min_len = strlen(needle);

	if (origin == NULL || needle == NULL || min_len == 0)
		return NULL;

	origin_len -= min_len;
	for (i = 0; i <= origin_len; i++)
	{
		if(0 == memcmp(origin+i, needle, min_len))
			return (char *)origin+i;
	}
	return NULL;
}

char *strchr2(const char *origin, int origin_len, char needle)
{
	int i;
	if (origin == NULL)
		return NULL;

	for (i = 0; i < origin_len; i++) {
		if (needle == origin[i])
			return (char *)origin +i;
	}
	return NULL;
}

char *strrcmp(const char *origin, int origin_len, const char *needle)
{
	int min_len = strlen(needle);
	const char *iter = origin + (origin_len - min_len);
	if (origin == NULL || needle == NULL || min_len == 0)
		return NULL;
	for (; iter >= origin; iter--) {
		if (0 == (memcmp(iter, needle, min_len)))
			return (char *)iter;
	}
	return NULL;
}

int str_end_with(const char *origin, const char *needle)
{
	int min_len = strlen(needle);
	int origin_len = strlen(origin);
	if (origin_len >= min_len && memcmp(origin + origin_len - min_len, needle, min_len) == 0)
		return 1;
	return 0;
}




#define swap_data(a, b) do{(a) ^= (b); (b) = (a) ^(b); (a) ^= (b);}while(0)

#define swap_arr(arr, arr_size) do {	\
	int index; (arr_size)--; \
	for (index = 0; index < (arr_size); index++, (arr_size)--) swap_data((arr)[index], (arr)[arr_size]); \
}while(0)

void swap_byte_arr(uint8_t arr[], int arr_size)
{
	swap_arr(arr, arr_size);
}

void swap_short_arr(uint16_t arr[], int arr_size)
{
	swap_arr(arr, arr_size);
}

void swap_int_arr(uint32_t arr[], int arr_size)
{
	swap_arr(arr, arr_size);

}

void swap_u64_arr(uint64_t arr[], int arr_size)
{
	swap_arr(arr, arr_size);
}



#if 0
void swap_bytes_unit(void *buff, unsigned int unit_num, short unit_size)
{
	unsigned int i, j;
	unit_num--; // to last index. 
	unsigned char *arr = (unsigned char *)buff;
	
	for (i = 0; i < unit_num; i++, unit_num--)
	{
		for (j = 0; j < unit_size; j++)
			swap_data(arr[i*unit_size +j], arr[unit_num*unit_size +j]);
	}
}
#endif

#if 0
int main(int argc, char **argv)
{
	char a[100];
	char *b = a + 10;
	memcpy(a, "12345678909", 12);
	memove2(b, a, 12);
	printf("%s\n", b);
	return 0;
}
#endif

uint32_t bytes_to_num(const uint8_t bytes[], int len, uint8_t littleendian)
{
	union 
	{
		uint32_t data;
		uint8_t bytes[4];
	}tmp = {
		0
	};

	if (littleendian) {
		switch (len)
		{
		case 4:
			tmp.bytes[3] = bytes[3];
		case 3:
			tmp.bytes[2] = bytes[2];
		case 2:
			tmp.bytes[1] = bytes[1];
		case 1:
			tmp.bytes[0] = bytes[0];
		default:
			break;
		}
	} else {
		switch (len)
		{
		case 4:
			tmp.bytes[3] = *bytes++;
		case 3:
			tmp.bytes[2] = *bytes++;
		case 2:
			tmp.bytes[1] = *bytes++;
		case 1:
			tmp.bytes[0] = *bytes++;
		default:
			break;
		}
	}
	return tmp.data;
}

uint32_t bytes_to_uint32(const uint8_t bytes[4], uint8_t littleendian)
{
	if (littleendian) {
		return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
	}
	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

uint16_t bytes_to_uint16(const     uint8_t bytes[2], uint8_t littleendian)
{
	if (littleendian) {
		return (bytes[1] << 8) | bytes[0];
	}
	return (bytes[0] << 8) | bytes[1];
}


void uint16_to_bytes(uint16_t num, uint8_t bytes[2], uint8_t littleendian)
{
	if (littleendian) {
		bytes[1] = num>>8;
		bytes[0] = num;
	} else {
		bytes[0] = num>>8;
		bytes[1] = num;
	}
}

void uint32_to_bytes(uint32_t num, uint8_t bytes[4], uint8_t littleendian)
{
	if (littleendian) {
		bytes[3] = num>>24;
		bytes[2] = num>>16;
		bytes[1] = num>>8;
		bytes[0] = num;
	}else{
		bytes[0] = num>>24;
		bytes[1] = num>>16;
		bytes[2] = num>>8;
		bytes[3] = num;
	}
}








