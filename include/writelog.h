#ifndef _WWRITELOG_H_
#define _WWRITELOG_H_

#include <stdlib.h>

#define KBYTE       1024

#define MAX_LOG_QUEUE_SIZE      10240
#define MAX_LOG_TEXT_SIZE       65534
#define MAX_LOG_FULLPATH_SIZE   2048
#define MAX_LOGNAME_SIZE        512
#define MAX_LOG_PATH_SIZE       1024

#define MAX_ARGBUF_SIZE         63000

#define FALSE       -1
#define TRUE        0

#define WLOGFREE(a) \
    if(a != NULL) free(a); \
    a = NULL;

#define LOG_DEBUG(args, ...) \
    do { \
        if(loglevel >= 3) { \
           logwrite("DEBUG", __FILE__, __LINE__, __FUNCTION__, args, ##__VA_ARGS__); \
        } \
    } while(0)

#define LOG_ERR(args, ...) \
    do { \
        if(loglevel >= 2) { \
            logwrite("ERR", __FILE__, __LINE__, __FUNCTION__, args, ##__VA_ARGS__); \
        } \
    } while(0) 

#define LOG_WARN(args, ...) \
    do { \
        if(loglevel >= 1) { \
            logwrite("WARN", __FILE__, __LINE__, __FUNCTION__, args, ##__VA_ARGS__); \
        } \
    } while(0) 

#define LOG_INFO(args, ...) \
    do { \
        if(loglevel >= 0) { \
            logwrite("INFO", __FILE__, __LINE__, __FUNCTION__, args, ##__VA_ARGS__); \
        } \
    } while(0) 

#define PACKETDUMP(p, sz)   pdump(p, sz);

typedef enum logset
{
    LOG_INFO = 0,
    LOG_WARN,
    LOG_ERROR,
    LOG_DEBUG,
    LOG_DUMP
} _logset;

typedef struct loginfo
{
    char dir[MAX_LOG_PATH_SIZE];
    char fname[MAX_LOGNAME_SIZE];
    char fullpath[MAX_LOG_FULLPATH_SIZE];
    int lfile;
} _loginfo_t;

typedef struct logq
{
    int max;
    int num;
    char *text[MAX_LOG_QUEUE_SIZE];
} logq_t;

extern _logset loglevel;

#ifdef __cplusplus
extern "C" {
#endif

/*  function    */
int init_log(_logset set, int max);
void *log_thread(void *arg);
int get_lque_size(const logq_t *que);
int fwrite_text();
int logfile_size_check();
int lotate_file();
int create_logfile(char *dir, char *name);

void pdump(unsigned char *packet, int len);

void destroy_log();
void terminate_log(logq_t *que);
void clear_lque(logq_t *que);

void change_log_level(_logset set);
void logwrite(const char *level, const char *filename, const int line, const char *funcname, const char * args, ...);
void add_logtext(char* newtext);
void print_lque (const logq_t *que);

#ifdef __cplusplus
}
#endif

#endif