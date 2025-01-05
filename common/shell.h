
/**********************************************************************************
 * FILE : shell.h
 * Description:
 * Author: Kevin He
 * Created On: 2018-12-02 , At 14:45:14
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_SHELL_H
#define KV_SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/************************ For Register console command ****************************/
typedef struct 
{
	const char *cmd;
	void (*func)(int argc, char * const argv[]);
}cmd_list_t;

#if 0
#define DECLAREE_CMD_FUNC(cmd, func) \
	cmd_list_t list_##func  __attribute__((used, section("cmdList"))) \
	= {cmd, func}
#endif

/**********************************************************************************/

void start_shell_task(void);

int do_shell_loop(void);

void _outbyte(int c);
int _inbyte(unsigned short timeout);
void debug_data(const char *name, const unsigned char *data, int len);

/********************************add for shell return******************************/
extern unsigned char cmd_return_log_lv;
#define CMD_RETURN(fmt,args...) do{if (cmd_return_log_lv) printf("\n[return:]" fmt, ##args);}while(0)


#ifdef __cplusplus
}
#endif

#endif
