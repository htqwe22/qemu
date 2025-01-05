/**********************************************************************************
 * FILE : simple_vsprintf.c
 * Description:
 * Author: Kevin He
 * Created On: 2024-04-17 , At 10:46:59
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "simple_vsprintf.h"

//#include <stdarg.h>
#include <stddef.h>

#define get_num_va_args(_args, _lcount)	 (_lcount > 1)  ?  	va_arg(_args, long long int):	\
										 (_lcount == 1) ? 	va_arg(_args, long int): \
										 					va_arg(_args, int)
										 
#define get_unum_va_args(_args, _lcount) (_lcount > 1)  ?  	va_arg(_args, unsigned long long int):	\
										 (_lcount == 1) ? 	va_arg(_args, unsigned long int): \
										 					va_arg(_args, unsigned int)


#if 0
/*
 * put_dec_full4 handles numbers in the range 0 <= r < 10000.
 * The multiplier 0xccd is round(2^15/10), and the approximation
 * r/10 == (r * 0xccd) >> 15 is exact for all r < 16389.
 */
static void put_dec_full4(char *end, unsigned int r)
{
	int i;

	for (i = 0; i < 3; i++) {
		unsigned int q = (r * 0xccd) >> 15;
		*--end = '0' + (r - q * 10);
		r = q;
	}
	*--end = '0' + r;
}

/* put_dec is copied from lib/vsprintf.c with small modifications */

/*
 * Call put_dec_full4 on x % 10000, return x / 10000.
 * The approximation x/10000 == (x * 0x346DC5D7) >> 43
 * holds for all x < 1,128,869,999.  The largest value this
 * helper will ever be asked to convert is 1,125,520,955.
 * (second call in the put_dec code, assuming n is all-ones).
 */
static unsigned int put_dec_helper4(char *end, unsigned int x)
{
	unsigned int q = (x * 0x346DC5D7ULL) >> 43;

	put_dec_full4(end, x - q * 10000);
	return q;
}



/* Based on code by Douglas W. Jones found at
 * <http://www.cs.uiowa.edu/~jones/bcd/decimal.html#sixtyfour>
 * (with permission from the author).
 * Performs no 64-bit division and hence should be fast on 32-bit machines.
 */

static char * put_dec(char *end, unsigned long long n)
{
	unsigned int d3, d2, d1, q, h;
	char *p = end;

	d1  = ((unsigned int)n >> 16); /* implicit "& 0xffff" */
	h   = (n >> 32);
	d2  = (h      ) & 0xffff;
	d3  = (h >> 16); /* implicit "& 0xffff" */

	/* n = 2^48 d3 + 2^32 d2 + 2^16 d1 + d0
	     = 281_4749_7671_0656 d3 + 42_9496_7296 d2 + 6_5536 d1 + d0 */
	q = 656 * d3 + 7296 * d2 + 5536 * d1 + ((unsigned int)n & 0xffff);
	q = put_dec_helper4(p, q);
	p -= 4;

	q += 7671 * d3 + 9496 * d2 + 6 * d1;
	q = put_dec_helper4(p, q);
	p -= 4;

	q += 4749 * d3 + 42 * d2;
	q = put_dec_helper4(p, q);
	p -= 4;

	q += 281 * d3;
	q = put_dec_helper4(p, q);
	p -= 4;

	put_dec_full4(p, q);
	p -= 4;

	/* strip off the extra 0's we printed */
	while (p < end && *p == '0')
		++p;

	return p;
}
#endif

static char *number(char *end, unsigned long long num, int base, char locase, int width)
{
	char *start = end;
//	int i;
	/*
	 * locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters
	 */

	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

	if (num != 0) {
		switch (base) {
		case 10:
			do {
				*--start = (num % 10) + '0';
				num = num / 10;
			}while (num != 0);
			break;
			
		case 8:
			do {
				*--start = '0' + (num & 07);
				num >>= 3;
			}while (num != 0);
			break;
			
		case 16:
			do {
				*--start = digits[num & 0xf] | locase;
				num >>= 4;
			}while (num != 0);
			break;
		default:
			// not spport
			break;
		}
	}else {
		*--start = '0';
	}
	width -= (end - start);	
	while (width-- > 0) {
		*--start = '0';
	}
	return start;
}


int simple_vsprintf(char *buf, const char *fmt, va_list va)
{
	char *str, *start;
	char num_buf[24]; // to store a 64bits num ...
	char * const end = num_buf + sizeof(num_buf);
	int l_count, locase;
	int padn;
	long long int num;
	unsigned long long unum;
	str = buf;
	for ( ; *fmt ; fmt++) {		
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}		
		l_count = 0;
		locase = 0;
		padn = 0;

		fmt++;
loop:
		switch (*fmt) {
		case '%':
			*str++ = '%';
			break;
		
		case 'i':
		case 'd':
			num = get_num_va_args(va, l_count);
			if (num < 0) {
				*str++ = '-';
				unum = (unsigned long long) -num;
				padn--;
			} else {
				unum = (unsigned long long) num;
			}
			start = number(end, unum, 10, 0, padn);
			while(start < end) {
				*str++ = *start++;
			}
			break;

		case 'u':
			unum = get_unum_va_args(va, l_count);
			start = number(end, unum, 10, 0, padn);
			while(start < end) {
				*str++ = *start++;
			}		
			break;
			
		case 's':
			start = va_arg(va, char *);
			while (*start) {
				*str++ = *start++;
			}
			break;


		case 'x':
			locase = 0x20;
		case 'X':
			unum = get_unum_va_args(va, l_count);
			start = number(end, unum, 16, locase, padn);
			
			while(start < end) {
				*str++ = *start++;
			}
			break;
			
		case 'p':
			*str++ = '0';
			*str++ = 'x';
			padn -= 2;
			unum = (unsigned long long)va_arg(va, void *);
			start = number(end, unum, 16, 0x20, padn);
			while(start < end) {
				*str++ = *start++;
			}
			break;	

		case 'c':
			*str++ = (unsigned char) va_arg(va, int);
			break;

		case 'o':
			unum = get_unum_va_args(va, l_count);
			start = number(end, unum, 8, 0, padn);
			while(start < end) {
				*str++ = *start++;
			}
			break;	
		case 'l':
			l_count++; 
			fmt++;
			goto loop;

		case 'z':
			if (sizeof(size_t) == 8)
				l_count = 2;
			fmt++;
			goto loop;
			
		case '#':
			*str++ = '0';
			*str++ = 'x';
//			padn -= 2;
			fmt++;
			goto loop;

		case '0':
			for (fmt++; *fmt >= '0' && *fmt <= '9'; fmt++) {
				padn = padn * 10 + (*fmt) - '0';
			}
			goto loop;
		
		default:
			break;
				
		}
	}
	*str = 0;
	return str - buf;
	return 0;	
}


