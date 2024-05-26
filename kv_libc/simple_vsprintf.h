/**********************************************************************************
 * FILE : simple_vsprintf.h
 * Description:
 * Author: Kevin He
 * Created On: 2024-04-17 , At 10:46:59
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/
#ifndef KV_SIMPLE_VSPRINTF_H
#define KV_SIMPLE_VSPRINTF_H
#include <stdarg.h>
#ifdef __cplusplus
extern "C" {
#endif

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)

int simple_vsprintf(char *buf, const char *fmt, va_list va);




#ifdef __cplusplus
}
#endif

#endif //simple_vsprintf.h