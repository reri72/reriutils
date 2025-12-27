#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>

#include "diskcalc.h"
#include "memcalc.h"
#include "cpucalc.h"

/* functions   */
void init_usage();
void init_arg(int count, char *values[]);
int init_monitor();
void n_sleep(int sec, int nsec);
void print_help();
void *monitor_thread(void *proc_id);
bool get_procstatus(int _pid);

/* variables   */
// 전역 변수들은 기존 코드를 따릅니다.
pid_t pid;
pthread_t tid;
bool run = false;

bool b_printcpu = false;
CpuContext ctx = {0,};

bool b_printmemory = false;

bool b_printdisk = false;

void sigHandle(int signum)
{
    printf("\n");
    printf("signal : %d , bye. \n", signum);

    run = false;

    exit(0);
}

void print_help()
{
    printf("Usage: \n");
    printf(" -p, --p : process pid \n");
    printf(" -c, --c : print cpu \n");
    printf(" -m, --m : print memory \n");
    printf(" -d, --d : print disk \n");
    printf(" -h, --h : print help \n");
    printf("\n");

    exit(0);
}

void n_sleep(int sec, int nsec)
{
    struct timespec rem, req;
    req.tv_sec = sec;
    req.tv_nsec = nsec;

    // nanosleep 호출 중 시그널에 의해 중단될 경우, 남은 시간을 다시 계산하여 슬립
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }

    return;
}

int main(int argc, char *argv[])
{
    struct sysinfo info;

    init_arg(argc, argv);
    init_usage();

    run = true;

    if (init_monitor() != 0) return -1;

#ifdef _DEBUG
    sysinfo(&info);
    printf("load info : %lu %lu %lu \n", info.loads[0], info.loads[1], info.loads[2] );
    printf("memory info : %lu %lu %lu \n", info.totalram, info.totalram-info.freeram, info.freeram );
#endif

    ctx._proc_t = pid;

    while (run)
    {
        if (b_printcpu)
        {
            if (update_cpu_usage(&ctx))
            {
                getProcinfo(&ctx);
                printf("System CPU: %0.1f%% | Process: %.1f%%\n",
                            ctx.system_total_usage, ctx.process_usage);
            }
        }

        if (b_printmemory)
        {
            Meminfo mems = {0,};
            pMeminfo pmems = {0,};
            pStatm pstatm = {0,};

            if (!readMemInfo(&mems))
            {
                printf(" '/proc/meminfo' read failed. \n");
            }
            else
            {
                double real_mem = ((double)mems.mtot - (double)mems.fmree) / (double)mems.mtot * 100.0;
                double pretext_mem = ((double)mems.mtot - (double)mems.mavail) / (double)mems.mtot * 100.0;

                printf("Linux real memory usage(%%) : %g %% \n", real_mem);
                printf("Linux pretext memory usage(%%) : %g %% \n", pretext_mem);
            }

            if (!ReadProcStatm(&pstatm, pid))
            {
                printf(" '/proc/%d/statm' read failed. \n", pid);
            }

            if (!readProcMemInfo(&pmems, pid))
            {
                printf(" '/proc/%d/status' read failed. \n", pid);
            }
            else
            {
                // VmRSS를 사용해 프로세스 메모리 사용률 계산
                if (mems.mtot > 0)
                {
                    double pmem = ( (double)pmems.VmRSS / (double)mems.mtot ) * 100.0;
                    printf("Process memory usage(%%) : %g %% \n", pmem);
                }
                else
                {
                    printf("Total memory is zero, cannot calculate process memory usage.\n");
                }
            }
        }

        if (b_printdisk)
        {
            Mountinfo *minfo = NULL;
            if ((minfo = MountOpen()) == NULL)
            {
                printf(" '/proc/mounts' open failed. \n");
            }
            else
            {
                while (ReadMountInfo(minfo))
                {
                    double disk_usage = CalculateDiskUsage(minfo->mountdir);
                    printf("mount: %s [%s], usage: %.2f%%\n", 
                            minfo->devname, minfo->mountdir, disk_usage);
                }
                MountReadClose(minfo);
            }
        }
        
        printf("--------------------------------\n\n");
        n_sleep(3, 0);
    }
    
    return 0;
}

void init_usage()
{
    signal(SIGINT, sigHandle);
    signal(SIGTERM, sigHandle);
}

void init_arg(int count, char *values[])
{
    int i = 0, j = 0;
    int opt;

    while( (opt = getopt(count, values, "hcmdp:")) != -1 )
    {
        switch(opt)
        {
            case 'h':
                print_help();
                break;
            case 'c':
                b_printcpu = true;
                break;
            case 'm':
                b_printmemory = true;
                break;
            case 'd':
                b_printdisk = true;
                break;
            case 'p':
                if (optarg)
                {
                    for (j = 0; optarg[j] != '\0'; j++)
                    {
                        if ( !isdigit(optarg[j]) )
                        {
                            printf("Error: PID must be a number.\n");
                            print_help();
                        }
                    }
                    pid = atoi(optarg);
                }
                break;

            case '?':
                print_help();
                break;
        }
    }
}

int init_monitor()
{
    pthread_t thr;
    pthread_attr_t attr;

    if (pid <= 0)
    {
        fprintf(stderr, "Process PID is not set. Cannot start monitor thread. \n");
        return -1;
    }

    if (pthread_attr_init(&attr) != 0)
    {
        fprintf(stderr, "pthread_attr_init failed. \n");
        return -1;
    }
    
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
    {
        fprintf(stderr, "pthread_attr_setdetachstate failed. \n");
        return -1;
    }

    if (pthread_create(&tid, &attr, monitor_thread, (void *)(long)pid) != 0)
    {
        fprintf(stderr, "pthread_create failed. \n");
        return -1;
    }

    if (pthread_attr_destroy(&attr) != 0)
    {
        fprintf(stderr, "pthread_attr_destroy failed. \n");
        return -1;
    }
    
    n_sleep(1, 0);
    
    return 0;
}

void *monitor_thread(void *proc_id)
{
    int monitor_pid = (int)(long)proc_id;

    printf("== Monitor Thread for PID %d started. == \n", monitor_pid);

    while (run)
    {
        bool ret = get_procstatus(monitor_pid);
        if (!ret)
        {
            printf("Process %d not alive. Monitor thread exiting. \n", monitor_pid);
            run = false;
        }
        n_sleep(2, 0);
    }

    printf("== Monitor Thread for PID %d exiting. == \n", monitor_pid);
    return NULL;
}

bool get_procstatus(int _pid)
{
    char buf[1024] = {0,};
    snprintf(buf, sizeof(buf), "/proc/%d", _pid);

    if (access(buf, F_OK) == 0) {
        return true;
    } else {
        return false;
    }
}
