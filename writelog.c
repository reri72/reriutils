#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

#include "sockC.h"
#include "writelog.h"

_logset     loglevel = LOG_ERROR;
logq_t      logqueue;
_loginfo_t  li;

pthread_t tid;
pthread_mutex_t _mutex;

volatile bool status = false;
pthread_cond_t log_cond = PTHREAD_COND_INITIALIZER;

void *log_thread(void *arg)
{
    while(status)
    {
        pthread_mutex_lock(&_mutex);
        {
            while (get_lque_size(&logqueue) == 0 && status)
            {
                pthread_cond_wait(&log_cond, &_mutex);
            }
        }
        pthread_mutex_unlock(&_mutex);

        if (li.lfile)
        {
            fwrite_text();

            if (logfile_size_check() > (KBYTE*KBYTE*10))
            {
                int res = lotate_file();
                if (res < 0)
                {
                    fprintf(stderr, "log rotation failed\n");
                }
            }
        }
    }

    return NULL;
}

int init_log(_logset set, int max)
{
    status = true;

    loglevel = set;
    
    logqueue.num = 0;
    if (max > MAX_LOG_QUEUE_SIZE)
        logqueue.max = MAX_LOG_QUEUE_SIZE;
    else
        logqueue.max = max;
    
    memset(&li, 0x00, sizeof(_loginfo_t));

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);  // re-entry

    pthread_mutex_init(&_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    if ( pthread_create(&tid, NULL, log_thread, NULL) != 0 )
    {
        fprintf(stderr, "pthread_createe failed\n");
        perror("thread create error");
    }

    if ( pthread_detach(tid) != 0 )
    {
        fprintf(stderr, "pthread_detach failed\n");
        perror("thread detach error");
    }

    return 0;
}

void terminate_log(logq_t *que)
{
    clear_lque(que);
    que->max = que->num = 0;
}

void destroy_log()
{
    pthread_mutex_destroy(&_mutex);

    status = false;
    pthread_cond_signal(&log_cond);
    
    nano_sleep(0, 100);

    pthread_mutex_destroy(&_mutex);
    terminate_log(&logqueue);

    close(li.lfile);
}

void change_log_level(_logset set)
{
    loglevel = set;
}

void logwrite(const char *level, const char *filename, const int line, const char *funcname, const char * args, ...)
{
    char tmstr[128] = {0,};
    char logbuffer[MAX_LOG_TEXT_SIZE] = {0,};
    char argbuf[MAX_ARGBUF_SIZE] = {0,};

    time_t now = time(NULL);
    struct tm ts;

    va_list va;

    va_start(va, args);
    vsnprintf(argbuf, sizeof(argbuf), args, va);
    va_end(va);

    localtime_r(&now, &ts);
    strftime(tmstr, sizeof(char) * 128, "%Y-%m-%d %H:%M:%S", &ts);

    if (strstr(level, "DUMP") != NULL)
        snprintf(logbuffer, sizeof(logbuffer), "%-20s [%s] : %s",
            tmstr, level, argbuf);
    else
        snprintf(logbuffer, sizeof(logbuffer), "%-20s [%s] %s %d %s : %s",
            tmstr, level, filename, line, funcname, argbuf);

    pthread_mutex_lock(&_mutex);
    {
        add_logtext(logbuffer);
    }
    pthread_mutex_unlock(&_mutex);
}

int create_logfile(char *dir, char *name)
{
    if (access(dir, F_OK) == -1)
    {
        if (mkdir(dir, 0755) < 0)
        {
            perror("Failed to create directory");
            return -1;
        }
    }

    snprintf(li.fullpath, sizeof(li.fullpath), "%s/%s", dir, name);
    li.lfile = open(li.fullpath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (li.lfile < 0)
    {
        perror("Failed to open log file");
        return -1;
    }
 
    if (li.lfile == -1)
    {
        perror("File open error");
        return -1;
    }

    return 0;
}

void pdump(unsigned char *packet, int len)
{
    char logbuf[MAX_LOG_TEXT_SIZE] = {0,};
    char *ptr = logbuf;
    int offset = 0;
    
    int i = 0;
    for (i = 0; i < len+1; i++)
    {
        offset += snprintf(ptr + offset, sizeof(logbuf) - offset, "%02X ", packet[i]);
        
        if ((i + 1) % 8 == 0)
            offset += snprintf(ptr + offset, sizeof(logbuf) - offset, " ");

        if ((i + 1) % 16 == 0)
            offset += snprintf(ptr + offset, sizeof(logbuf) - offset, "\n");

        if (offset >= sizeof(logbuf) - 6) // 버퍼 오버플로 방지
            break;
    }

    logwrite("DUMP", __FILE__, __LINE__, __FUNCTION__, "\n%s\n\n", logbuf);
}

int lotate_file()
{
    char nname[MAX_LOG_FULLPATH_SIZE+3] = {0,};
    char nname2[MAX_LOG_FULLPATH_SIZE+3] = {0,};
    char nfname[MAX_LOG_FULLPATH_SIZE+3] = {0,};

    int i;
    
    close(li.lfile);

    for(i = 9; i > 0; i--)
    {
        snprintf(nname, sizeof(nname), "%s%s.%d", li.dir, li.fname, i);
        snprintf(nname2, sizeof(nname2), "%s%s.%d", li.dir, li.fname, (i+1));
        rename(nname, nname2);

        nname2[0] = 0;
        nname[0] = 0;
    }

    snprintf(nfname, sizeof(nfname), "%s.%d", li.fullpath, 1);
    rename(li.fullpath, nfname);

    int ret = create_logfile(li.dir, li.fname);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

int fwrite_text()
{
    int ret = TRUE;

    if (li.lfile)
    {
        pthread_mutex_lock(&_mutex);
        {
            if (logqueue.num > 0)
            {
                int i;
                for(i = 0; i < logqueue.num; i++)
                {
                    write(li.lfile, (const char*)logqueue.text[i], strlen(logqueue.text[i]));
                }
                clear_lque(&logqueue);
            }
        }
        pthread_mutex_unlock(&_mutex);
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

int logfile_size_check()
{
    struct stat finfo;
    
    if (stat(li.fullpath, &finfo) < 0)
    {
        // printf("[%s] log file size check error. \n", __FUNCTION__);
        perror("stat");
        // exit(1);
    }

    return finfo.st_size;   // Byte
}

int get_lque_size(const logq_t *que)
{
    return que->num;
}

void print_lque (const logq_t *que)
{
    if (que->num > 0)
    {
        int i;
        for(i = 0; i < que->num; i++)
        {
            printf("queue[%d] : %s \n", i, que->text[i]);
        }
    }
}

void add_logtext(char* newtext)
{
    pthread_mutex_lock(&_mutex);
    {
        if (logqueue.num < logqueue.max)
        {
            if (logqueue.num >= logqueue.max)
            {
                WLOGFREE(logqueue.text[0]);
                memmove(&logqueue.text[0], &logqueue.text[1], sizeof(char*) * (logqueue.num-1));
                logqueue.num--;
            }

            logqueue.text[logqueue.num] = strdup(newtext);
            if (logqueue.text[logqueue.num] != NULL)
            {
                logqueue.num++;
                pthread_cond_signal(&log_cond);
            }
            else
            {
                fprintf(stderr, "memory allocation failure\n");
            }
        }
        else
        {
            fprintf(stderr, "log queue is full\n");
        }
    }
    pthread_mutex_unlock(&_mutex);
}

void clear_lque(logq_t *que)
{
    for (int i = 0; i < que->num; i++)
    {
        WLOGFREE(que->text[i]);
    }
    que->num = 0;
}
