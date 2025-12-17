#include "cpucalc.h"

unsigned long long total_cpu_usage;
unsigned long total_proc_time;
double pcpu_usage;
double cpu_usage;

void getSystemStat()
{
    FILE * fp = fopen( "/proc/stat", "r");
    char buf[BUFSIZ] = {0,};

    static unsigned long long prev_total = 0;
    static unsigned long long prev_idle = 0;

    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;

    if (fp != NULL)
    {
        if (fgets(buf, BUFSIZ, fp))
        {
            // CPU의 전체 시간 = 이 모든 필드의 합
            sscanf(buf, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &user, &nice, &system, &idle,
                    &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

            // 보통 guest, guest_nice는 제외해도 됨 (가상화 환경이 아니면 0)
            unsigned long long cur_idle = idle + iowait;
            unsigned long long cur_total = user + nice + system + idle + iowait + irq + softirq + steal;

            unsigned long long delta_total = cur_total - prev_total;
            unsigned long long delta_idle = cur_idle - prev_idle;


            if (delta_total > 0)
                cpu_usage = 100.0 * (delta_total - delta_idle) / delta_total;

            total_cpu_usage = delta_total;

            prev_total = cur_total;
            prev_idle  = cur_idle;
        }

        fclose(fp);
    }
    else
    {
        fprintf(stderr, " '/proc/stat' file open failed. \n");
    }
    
    return;
}

void getProcinfo(pid_t pid)
{
    char ppath[BUFSIZ] = {0,};
    int cx = 0;

    cx = snprintf( ppath, sizeof(ppath), "/proc/%d/stat", pid);
    if (!cx)
    {
        return;
    }

    FILE * fp1 = fopen( ppath, "r");

    char buf1[BUFSIZ] = {0,};

    unsigned long pUtime, pStime;
    unsigned long pCutime, pCstime;
    static unsigned long tproc_time, btproc_time;

    if (fp1 != NULL)
    {
        fgets(buf1, BUFSIZ, fp1);

        //14(utime), 15(stime)
        //16(Cutime), 17(Cstime)
        sscanf(buf1, "%*d %*s %*c %*d %*d"
                    "%*d %*d %*d %*u %*u"
                    "%*u %*u %*u %lu %lu"
                    "%ld %ld %*d %*d %*d"
                    , &pUtime, &pStime, &pCutime, &pCstime);

        tproc_time = pUtime + pStime + pCutime + pCstime;

        total_proc_time = tproc_time - btproc_time;
        btproc_time = tproc_time;

        fclose(fp1);
    }
    else
    {
        fprintf(stderr, " '/proc/pid/stat' file open failed. \n");
        total_proc_time = 0;
    }
    
    return;
}

bool calCpu()
{
#if 0
    // irix mode (단일 코어를 기준 계산)
    pcpu_usage = CPU_EA * total_proc_time * 100 / (float) total_cpu_usage ;
#else
    // solaris mode (전체 코어 수를 기준으로 계산)
    pcpu_usage = total_proc_time * 100 / (float) total_cpu_usage ;
#endif

    if (pcpu_usage < 0)
    {
        return false;
    }

    return true;
}

void getSystemUptime()
{
    FILE * fp = fopen( "/proc/uptime", "r");
    char buf[BUFSIZ] = {0,};
    double u1 = 0, u2 = 0;

    if (fp != NULL)
    {
        fgets(buf, BUFSIZ, fp);

        sscanf(buf, "%lf %lf", &u1, &u2);
        
        fclose(fp);
    }
    else
    {
        fprintf(stderr, " '/proc/uptime' file open failed. \n");
    }

    return;
}