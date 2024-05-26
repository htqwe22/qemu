/**********************************************************************************
 * FILE : log.c
 * Description:
 * Author: 
 * Created On: 2022-10-12 , At 19:47:34
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "log.h"
#include <string.h>
#include "simple_vsprintf.h"

int g_log_level = DEFUALT_LOG_THRESHOLD;

const char * log_lv[] = 
{
	"",
	"ERR",
	"WARN",
	"I",
	"D",
};


const char *_basename(const char *path)
{
#ifdef WIN32
#define DIR_SPILT_CHAR	'\\'
#else
#define DIR_SPILT_CHAR	'/'
#endif
	const char *ptr = strrchr(path, DIR_SPILT_CHAR);
	return ptr ? (ptr + 1) : path;
}


void kv_debug_data(const char *name, const unsigned char *data, int len)
{
#if USE_BUFF_PRINT
	int buff_len = len * 3; // every byte use 3 bytes to print. 
	buff_len += (len >>5) + 4; // give one \n for every 32 bytes and 4 bytes reserved.
	char * print_buff = (char *)malloc(buff_len + 20);

	int i;
	buff_len = snprintf(print_buff, 20, "[%s] (%d)", name, len);
	if (print_buff) {
//		buff_len = 0;
		for (i = 0; i < len; i++) {
			if ((i & 0x1f) == 0)
				buff_len += sprintf(print_buff + buff_len, "\n");
			buff_len += sprintf(print_buff + buff_len, "%02X ", data[i]);
		}
	}
	buff_len += sprintf(print_buff + buff_len, "\n");
	log_write(print_buff, buff_len);
	free(print_buff);
#else
	int i;
	if ((LOG_LVL_DEBUG) <= g_log_level) {
		kv_printf("[%s] (%d)", name, len);
		for (i = 0; i < len; i++)
		{
			if ((i & 0x1f) == 0)
				kv_printf("\n");
			kv_printf("%02X ", data[i]);
		}
		kv_printf("\n");
	}
#endif
}


void kv_debug_str(const char *name, const char *str, int len)
{
	if ((LOG_LVL_DEBUG) <= g_log_level) {
		kv_printf( "[%s] (%d)\n", name, len);
		kv_printf( "%.*s", len, str);	
		kv_printf( "\n");
	}
}


void set_debug_level(int lvl)
{
	if (lvl > 0 && lvl <= LOG_LVL_DEBUG)
		g_log_level = lvl;
}

extern void uart_send(char c);
int kv_printf(const char *format, ...)
{

	int len;
	char buffer[PRINT_BUFFER_SIZE];
	va_list va;

	va_start(va, format);
	len = simple_vsprintf(buffer, format, va);
	va_end(va);
	int i;

	for (i = 0; i < len; i++) {
		if (buffer[i] == '\n')
			uart_send('\r');
		uart_send(buffer[i]);
	}
	return len;
}