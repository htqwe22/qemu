/**********************************************************************************
 * FILE : log.h
 * Description:
 * Author: 
 * Created On: 2022-10-12 , At 19:47:34
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_DV_LOG_H
#define KV_DV_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_ENABLE     1

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif


#define PRINT_BUFFER_SIZE 200

#define COLOR_NONE         "\033[0m"      
#define COLOR_RED          "\033[0;32;31m"
#define COLOR_LIGHT_RED    "\033[1;32;32m"

#define COLOR_LIGHT		   "\033[1;31m"

#define COLOR_GREEN        "\033[0;32m"   
#define COLOR_BLUE         "\033[0;32;34m"
#define COLOR_DARK_GRAY    "\033[1;30m"   
#define COLOR_CYAN         "\033[0;36m"   
#define COLOR_PURPLE       "\033[0;35m"   
#define COLOR_BROWN        "\033[0;33m"   
#define COLOR_YELLOW       "\033[5;42;33m"
#define COLOR_WHITE        "\033[1;37m"	

#define LOG_LVL_ERR 	1
#define LOG_LVL_WARN	2
#define LOG_LVL_INFO	3
#define LOG_LVL_DEBUG	4

#if LOG_ENABLE

extern const char * log_lv[];
extern const char *_basename(const char *path);
extern int g_log_level;

#define DEFUALT_LOG_THRESHOLD	LOG_LVL_DEBUG
void set_debug_level(int lvl);

void kv_debug_data(const char *name, const unsigned char *data, int len);

void kv_debug_str(const char *name, const char *str, int len);

extern int kv_printf(const char *format, ...);

#define kv_debug(lvl, fmt, ...) 	if ((lvl) <= g_log_level) kv_printf("[%s:%d][%s]"fmt,  _basename(__FILE__), __LINE__, log_lv[lvl],##__VA_ARGS__)

#define kv_debug_raw(lvl, fmt, ...) if ((lvl) <= g_log_level) kv_printf(fmt, ##__VA_ARGS__) 


#define LOG_DEBUG(fmt, ...) 	if ((LOG_LVL_DEBUG) <= g_log_level) kv_printf("[%s:%d][%s]" fmt,  _basename(__FILE__), __LINE__, "D", ##__VA_ARGS__);
#define LOG_INFO(fmt, ...) 		if ((LOG_LVL_INFO) <= g_log_level) kv_printf(COLOR_CYAN "[%s:%d][%s]" fmt COLOR_NONE,  _basename(__FILE__), __LINE__, "I", ##__VA_ARGS__);
#define LOG_WARN(fmt, ...) 		if ((LOG_LVL_WARN) <= g_log_level) kv_printf(COLOR_BROWN "[%s:%d][%s]" fmt COLOR_NONE,  _basename(__FILE__), __LINE__, "WARN", ##__VA_ARGS__);
#define LOG_ERROR(fmt, ...) 	if ((LOG_LVL_ERR) <= g_log_level) kv_printf(COLOR_RED"[%s:%d][%s]" fmt COLOR_NONE ,  _basename(__FILE__), __LINE__, "ERR", ##__VA_ARGS__);

//#define debug_str(lvl, n, str) 	if ((lvl) <= g_log_level) kv_printf("[%s][%s:%d]%.*s\n",log_lv[lvl], _basename(__FILE__), __LINE__, n + 1, str)

#else

#define kv_debug_data(name, data, len) {(void)name, (void)data, (void) len;}

#define kv_debug_str(name, str, len) {(void)name, (void)str, (void) len;}

#define set_debug_level(lvl) {(void)(lvl);}

#define kv_printf(fmt, ...) {(void)fmt;}
#define LOG_DEBUG(fmt, ...) {(void)fmt;}
#define LOG_INFO(fmt, ...)  {(void)fmt;}
#define LOG_WARN(fmt, ...)  {(void)fmt;}
#define LOG_ERROR(fmt, ...) {(void)fmt;}
#define kv_debug_raw(lvl, fmt, ...) 
#endif



#ifdef __cplusplus
}
#endif

#endif //log.h
