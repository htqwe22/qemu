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

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#include <stdarg.h>
#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#else
typedef char * va_list;
#define  _AUPBND                (sizeof (int) - 1)
#define  _ADNBND                (sizeof (int) - 1)
#define _bnd(X, bnd)            (((sizeof (X)) + (bnd)) & (~(bnd)))

#define va_arg(ap, T)           (*(T *)(((ap) += (_bnd (T, _AUPBND))) - (_bnd (T,_ADNBND))))
#define va_start(ap, A)        ((ap) = (((char *) &(A)) + (_bnd (A,_AUPBND))))
#define va_end(ap)              (void) 0
#endif


int simple_vsprintf(char *buf, const char *fmt, va_list va);




#ifdef __cplusplus
}
#endif

#endif //simple_vsprintf.h