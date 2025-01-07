
/**********************************************************************************
 * FILE : shell.c
 * Description:
 * Author: Kevin He
 * Created On: 2018-12-02 , At 14:45:14
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#include "shell.h"
#include <stdint.h>
#include <stdlib.h>
#include <kv_string.h>
#include "log.h"
#include <arch_helpers.h>

/*******************************************************************/

#define module_strcmp	kv_strcmp
#define module_memset	kv_memset
#define module_strlen	kv_strlen
#define module_strcat	kv_strcat
#define console_printf  kv_printf
#define kv_strlen       strlen2
#define kv_strcmp		strcmp2
#define kv_memcpy		memcmp2
extern int pl011_getc(void);
extern int pl011_putc(char ch);
#define get_char		pl011_getc
#define put_char 		pl011_putc


#define OPS_RECORD_NUM		1
#define LOGI kv_printf

static void send_cr(int argc, char * const argv[]);
//static void do_xmodem(int argc, char * const argv[]);
static void do_mem_show(int argc, char * const argv[]);
static void do_go(int argc, char * const argv[]);
static void do_reboot(int argc, char * const argv[]);
static void do_read_el(int argc, char * const argv[]);
static void do_reg_ops(int argc, char * const argv[]);

cmd_list_t cmdlist_arr[] = 
{
	{"clear", send_cr},
//	{"help", print_help},
//	{"xmodem", do_xmodem},
	{"dump", do_mem_show},
	{"reg", do_reg_ops},
	{"read_el", do_read_el},
	{"go", do_go},
	{"reboot", do_reboot},
};



#define LIST_START() cmdlist_arr
#define LIST_NUM()	ARRAY_SIZE(cmdlist_arr)


#include <ctype.h>
#include <errno.h>
#include <limits.h>


/*********************************************************************************/

#if 0
#define MAX_PRINT_CACHE_SIZE 200
/* 
 * this is used to print in any place.
 */
int force_printf(const char *fmt, ...)
{
	va_list ap;
	int i;
	char str[MAX_PRINT_CACHE_SIZE];
	va_start(ap, fmt);
	i = vsnprintf(str, MAX_PRINT_CACHE_SIZE, fmt, ap);
	va_end(ap);
	return drv_uart_send(OFO_UART_DEBUG, (const unsigned char * )str, i);
}
#endif



//xmodem, ′??ú??á?1?ó?buffer
//char xbuff[1128] = {0}; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */

#if 0
static char stdout_off = 0;
void stdout_open(void)
{
	stdout_off = 0;
}
static void stdout_close(void)
{
	stdout_off = 1;
}
#endif




/******************************** For shell ***********************************************/
#define next_line() console_printf("\r\n") 
#if 0
static void print_help(int argc, char * const argv[])
{
	int i;
	if (argc != 1)
		return;
	console_printf("Usage Cmd --help\r\n");
	const cmd_list_t *plist = LIST_START();
	for(i = 0; i < LIST_NUM(); i++, plist++){
		console_printf("%s -- help\r\n",plist->cmd);
	}
}
#endif	

static void send_cr(int argc, char * const argv[])
{
	if (argc == 1)
		console_printf("\033[2J\033[1H");
}

unsigned from_big_endian(uint8_t *data, unsigned len)
{
	unsigned num;
	for (num = 0; len; len--, data++) {
		num <<= 8;
		num |= *data;
	}
	return num;
}

void do_mem_show (int argc, char * const argv[])
{
	uint8_t *addr;
	unsigned long length;
	if (argc != 3){
		console_printf("useage dump <addr> <length>\n");
		return ;
	}
	addr = (uint8_t *)strtoul2(argv[1], NULL, 0);
	length = strtoul2(argv[2], NULL, 0);
	kv_debug_data(argv[1], addr, length);
}


static void do_go(int argc, char * const argv[])
{
	unsigned long addr, length;
	uint8_t tmp[4];
	addr = 0x40000000;
	if (argc != 2){
		console_printf("useage go <addr>\n");		
		console_printf("use default addr 0x40000000\n");
	}else{
		length = hex_string_to_bin(tmp, 4, argv[1], kv_strlen(argv[1]));
		addr = from_big_endian(tmp, length);
	}
	kv_printf ("## Starting application at 0x%08lX ...\n", addr);
	((void (*)(void))addr)();
}

static void do_reboot(int argc, char * const argv[])
{

//	console_printf("rebooting...");
	while(1)
	{
	}
}

static void do_read_el(int argc, char * const argv[])
{
	uint64_t el = get_current_el();
	console_printf("current EL:%d\n", el);
}


/********************************** Task ************************************************/
//#define ALLOW_ARROW
#ifdef ALLOW_ARROW
struct cmd_history_node
{
	uint16_t len;
	char cmd[62];
};

static struct cmd_history_node hisory[OPS_RECORD_NUM];
static uint8_t arrow_len;
#endif

static void exec_cmd(char *const cmd)
{
	const cmd_list_t *list;
	int index;
	char *argv[10];
	int i =0,j=0,flag=0;
//	next_line();
	if(*cmd < ' ' || *cmd > '~') return;
	
	argv[j++] = cmd;
	for(i=0;cmd[i];i++,flag = 0){
		if(cmd[i] == ' ' || cmd[i] == '\t'){
			cmd[i++]=0;
			flag = 1;
		}	
		if(flag && cmd[i] && cmd[i] != ' ' && cmd[i] != '\t') {
			if(j + 1 <ARRAY_SIZE(argv)){ 
				argv[j++]=cmd+i;
			}else{
				break;
			}
		}	
	}
	argv[j] = NULL;
	for(list = LIST_START(), index = 0; index < LIST_NUM(); index++, list++)
	{
		if(list->func && 0 == module_strcmp(argv[0],list->cmd)){
			list->func(j, (char **)argv);
			break;
		}
	}
	if(index == LIST_NUM()){
		console_printf(" \"%s\" not support now",argv[0]);
	}
	next_line();
}

//#define TEST_UART 
int do_shell_loop(void)
{
	unsigned short i=0;
	int ch;
	char xbuff[128] = {0};
	console_printf("# ");
	/*get one byte ,it may block */
while (1) {
	ch = get_char();
	if (ch < 0)  {
		return -1 ;
	}
#ifdef TEST_UART
	put_char(ch);
	continue;
#else	

#ifdef ALLOW_ARROW
	if (arrow_len || ch == 0x1B)
	{
		if (ch == 0x1B) {
			arrow_len = 1;
			return 0;
		}
		if (arrow_len == 1 && ch == 0x5B) {\
			arrow_len = 2;
			return 0;
		}
		if (arrow_len == 2 && (ch == 0x41 || ch == 0x42 || ch == 0x43 || ch == 0x44)) {

			kv_memcpy(xbuff, hisory[0].cmd, hisory[0].len + 1);
			i =  hisory[0].len;
			console_printf("\r# %s", xbuff);
			arrow_len = 0;
			return 0;
		}
	}
#endif
	if(ch == '\r'){
#ifdef ALLOW_ARROW		
		if (i) {
			hisory[0].len = i;
			kv_memcpy(hisory[0].cmd, xbuff, i);
			hisory[0].cmd[i] = 0;
		}
#endif	
//		if(i !=0)
			xbuff[i]=0;
		console_printf("\n");
		exec_cmd(xbuff);
		console_printf("# ");
		i=0;
	}else{
		if (ch == '\b') {
			if(i > 0){
				xbuff[--i] = 0;
				put_char('\b');
				put_char(' ');
				put_char('\b');
//				console_printf("\b \b");
			} 			
		} else if (i + 1 < sizeof(xbuff)){
			xbuff[i++]=(char)ch;
			put_char(ch);
//			console_printf("%c", ch);
		}	
	}
#endif
}
	return 0;
}

#if 0
static void check_user_cmd(void)
{
	const char *cmd1, *cmd2;
	cmd_list_t *list = LIST_START();
	int i,j, num, list_num = LIST_NUM();
	for (i = 0; i < list_num; i++)
	{
		cmd1 = list[i].cmd;
		num = 0;
		for (j = i + 1; j < list_num; j++)
		{
			cmd2 = list[j].cmd;
			if (0 == module_strcmp(cmd1, cmd2))
			{
				num++;
			}
		}
		if (num)
		{
			printf("######## user has registered %d same command: \"%s\" ############\n", num +1, cmd1);
		}
	}
}
#endif

/****************************** The following is xmode *****************************/
#if 0

#define KV_CNTLQ       0x11
#define KV_CNTLS       0x13
#define KV_DEL         0x7F
#define KV_BACKSPACE   0x08
#define KV_CR          0x0D
#define KV_LF          0x0A
#define KV_ESC         0x1B


#define KV_XMODEM_SOH		0x01
#define KV_XMODEM_STX		0x02
#define KV_XMODEM_EOT		0x04
#define KV_XMODEM_ACK		0x06
#define KV_XMODEM_NAK		0x15
#define KV_XMODEM_CAN		0x18


#define XMODEM_SYNC_SOH_DATA_SIZE 	128
#define XMODEM_SYNC_STX_DATA_SIZE 	1024

typedef enum
{
	XMODEM_STD = 0,
	XMODEM_CRC,
	XMODEM_E1K,
	XMODEM_E1G
}XMODEM_TYPE;

#define MAX_SYNC_TIME	100
#define MAXRETRANS		16
#define MAX_ERROR_TIME	10
#define XMODEM_SYNC_SOH_DATA_SIZE 	128
#define XMODEM_SYNC_STX_DATA_SIZE 	1024
#define READCHAR_TIMEOUT	0x400  



void udelay(uint32_t time)
{
	for (;time;time--)
		tick_count(13500);
//		tick_count(135000);
}

static void xmodem_putc(char ch)
{
	fputc(ch, stdout);
}

static int xmodem_getc(int *c,unsigned timeout_ms)
{
#define DELAY 2
	int ch;
	unsigned counter;
	timeout_ms = timeout_ms*1000;	// to us
//	kv_printf("timout %d\n", timeout_ms);
	for (counter = 0; counter < timeout_ms; )
	{
		ch = get_char();
		if (ch < 0)
		{
			udelay(DELAY);
			counter += 100;			
//			kv_printf("now %d\n", counter);
		}else{
			*c = ch;
//			kv_printf("Get char\n");
			return 1; //True
		}
	}
//	kv_printf("Get char timeout\n");
	return 0;
}

static void flushinput(void)
{
	int data;
	while (xmodem_getc(&data,100))
		;
}
static void xmodem_cancel(void)
{
	flushinput();
	xmodem_putc(KV_XMODEM_CAN);
	xmodem_putc(KV_XMODEM_CAN);
	xmodem_putc(KV_XMODEM_CAN);	
}

//#define DEBUG_XMODE
#ifdef DEBUG_XMODE
static char xmode_info[2048];
short log_index=0;
#define record_reset() kv_memset(xmode_info,0,sizeof(xmode_info))
#define record_log(fmt,...) do{log_index+=kv_vsprintf(xmode_info+log_index,fmt,##__VA_ARGS__);}while(0)
#define show_log() kv_printf("%s\r\n",xmode_info);
#else
#define record_reset() 
#define record_log(...) 
#define show_log() 
#endif

extern unsigned short xmodem_crc16(const void *buf, int count,int flag);
static int compare_check(const unsigned char *buf, int sz,int flag)
{
	unsigned short result = xmodem_crc16(buf,sz,flag);
	record_log("rst:%04x,d1:%02x,d2:%02x",result,buf[sz],buf[sz+1]);
	if((buf[sz]<<8)+buf[sz+1] != result)
		return 0;
	return 1;
}

static int xmodemReceive(unsigned long dest, XMODEM_TYPE Type)
{	
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	unsigned char *p;
	unsigned char trychar = 'C';
	unsigned char packetno = 1; // packet sequence
	record_reset();
	unsigned file_len=0;
	unsigned short i,data_size;
	int retrans = MAXRETRANS,recv_data;
	// CRC FLAG = 0 is checksum, = 1 is crc16
	unsigned char crc_flag,err_cnt = 0;	
	int error_code=0;
	switch(Type)
	{
		case XMODEM_STD:
			trychar = KV_XMODEM_NAK;
			crc_flag = 0;
			break;
		case XMODEM_CRC:
		case XMODEM_E1K:
			trychar = 'C';
			crc_flag = 1;
			break;
		case XMODEM_E1G:
			trychar = 'G';
			crc_flag = 1;
			break;
		default:
			trychar = KV_XMODEM_NAK;
			crc_flag = 0;
			break;	 
	}
	for(;;){
		for (i=0;i<MAX_SYNC_TIME;i++) {
			if(trychar)
				xmodem_putc(trychar);
			if((xmodem_getc(&recv_data,READCHAR_TIMEOUT))){
				switch(recv_data){
					case KV_XMODEM_SOH:
					{
						data_size = XMODEM_SYNC_SOH_DATA_SIZE;
						record_log("KV_XMODEM_SOH ;");
						goto start_recv;
					}
					case KV_XMODEM_STX:
					{
						data_size = XMODEM_SYNC_STX_DATA_SIZE;
						record_log("KV_XMODEM_STX ;");
						goto start_recv;					
					}
					case KV_XMODEM_EOT:// normal end
					{
						xmodem_putc(KV_XMODEM_ACK);
						record_log("KV_XMODEM_EOT ;");
						flushinput();
						// udelay(100);
						return file_len;
					}
					case KV_XMODEM_CAN:
					{
						record_log("KV_XMODEM_CAN ;");
						xmodem_cancel();
						return -1;
						/*
						if(xmodem_getc(&recv_data,READCHAR_TIMEOUT)){
							xmodem_putc(KV_XMODEM_ACK);
							flushinput();
							// Canceled by remote
							return -1;
						}*/
					}
					default:
						break;
				}
			}
		}
		// IF HOST NOT SUPPORT CRC
		// not receive what I want.
		if (trychar == 'C') { trychar = KV_XMODEM_NAK; continue; }
		if(trychar == KV_XMODEM_NAK){
			xmodem_cancel();
			return -2;
		}
start_recv:
	if (trychar == 'C') crc_flag = 1;
	trychar = 0; // Do not send sync charactricter any more.
	p = xbuff;
	/*Get the sync data.*/
	*p++ = (char)recv_data;
	for (i = 0;  i < (data_size+(crc_flag?1:0)+3); ++i) {
		if (xmodem_getc(&recv_data,READCHAR_TIMEOUT)==0) goto reject;
		*p++ = (char)recv_data;
	}
	// Check the length
	record_log("d[1]:%02x,d2:%02x,pckno:%d ;",xbuff[1],xbuff[2],packetno);
	if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
		(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
		compare_check(&xbuff[3], data_size,crc_flag)) {
		if (xbuff[1] == packetno)	{
			p = (char *)dest;
			kv_memcpy (p+file_len, &xbuff[3], data_size);
			++packetno;
			file_len += data_size;
			retrans = MAXRETRANS+1;
		}
		// else xbuff[1] == packetno-1 ,the pre packet
		if (--retrans <= 0) {
			flushinput();
			xmodem_cancel();
			return -3; /* too many retry error */
		}
		xmodem_putc(KV_XMODEM_ACK);
		continue;
	}			

reject:
#if 0
	xmodem_cancel();
	printf("xmode error is %d\r\n",error_code);
	return 7;
#else	
	flushinput();
	// too many rx error, transmit abort
	if (err_cnt++ > MAX_ERROR_TIME)
	{
		xmodem_putc(KV_XMODEM_CAN);
		xmodem_putc(KV_XMODEM_CAN);
		xmodem_putc(KV_XMODEM_CAN);
		error_code = 5;
		return 0;			
	}
	else // re-send the packet
	{
		xmodem_putc(KV_XMODEM_NAK);
	}
#endif	
	}
}


static void do_xmodem(int argc, char * const argv[])
{
	unsigned int recv_addr;
	uint8_t tmp[4];
	int ret_len = 0, len;
	recv_addr = 0x40000000;
	if (argc != 2){
		console_printf("useage xmode <addr>\n");
		console_printf("use default addr 0x40000000\n");
	}else{
		len = hex_string_to_bin(argv[1], kv_strlen(argv[1]), tmp, 4);
		recv_addr = from_big_endian(tmp, len);
	}
	show_led(1);
	debug("load to 0x%08X\n", recv_addr);
	ret_len = xmodemReceive(recv_addr, XMODEM_E1K);	
	show_led(0);
	console_printf("over:");
	xmodem_getc(&len,30000);
	show_log();
	console_printf(" get ret_len = %d\r\n",ret_len);
}
#endif


#if 0

void _outbyte(int c)
{
	fputc((char)c, stdout);
}

int _inbyte(unsigned short timeout) // msec timeout
{
    unsigned char ch;
		
//	if (drv_uart_wait_rx(OFO_UART_DEBUG, timeout) >= 0 && drv_uart_recv(OFO_UART_DEBUG, &ch, 1) == 1) {
//			return ch;
//	}
	return -2;
}


static void xmodem_recv(int argc, char **argv)
{
	char fullName[13] = "0:/";

	if ((argc < 2) || NULL == argv[1])
	{
		return;
	}
	
	if (0 != module_strcmp(argv[0], "xmodem"))
	{
		return;
	}

	if (module_strlen(argv[1]) > 12)
	{
		console_printf("file name too long:%d\r\n", module_strlen(argv[1]));
		return;
	}

	module_strcat(fullName, argv[1]);
	console_printf("xmodem -> [%s]\r\n", fullName);

	stdout_close();
	xmodemReceive(fullName);

	//xmodem ?áê???1?ó??o′?????￡?±ü?a2Dá?êy?Yó°?ì?üá?DD′|àí
	module_memset(xbuff, 0, sizeof(xbuff));
	
	stdout_open();
}
#endif

void debug_data(const char *name, const uint8_t *data, int len)
{
	int i;
	LOGI("[%s] (%d)", name, len);
	for (i = 0; i < len; i++)
	{
		if ((i&0xf) == 0)
			LOGI("\n");
		LOGI("%02X ", data[i]);
	}
	LOGI("\n");
}




