#include "cpucalc.h"

bool update_cpu_usage(CpuContext *ctx)
{
    FILE *fp = fopen( "/proc/stat", "r");
    if (!fp)
        return false;
    
    char buf[BUFSIZ] = {0,};
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;

    if (fgets(buf, BUFSIZ, fp))
    {
        // CPU의 전체 시간 = 이 모든 필드의 합
        sscanf(buf, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &user, &nice, &system, &idle,
                &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

        // 보통 guest, guest_nice는 제외해도 됨 (가상화 환경이 아니면 0)
        // 리눅스 커널에서 user와 nice 값에는 이미 guest와 guest_nice 시간이 포함되어 있음
        unsigned long long cur_idle = idle + iowait;
        unsigned long long cur_total = user + nice + system + idle + iowait + irq + softirq + steal;

        if (ctx->_prev_total > 0)
        {
            unsigned long long delta_total = cur_total - ctx->_prev_total;
            unsigned long long delta_idle = cur_idle - ctx->_prev_idle;
            ctx->system_total_usage = (double)(delta_total-delta_idle) / delta_total * 100.0;
        }

        ctx->_prev_total = cur_total;
        ctx->_prev_idle = cur_idle;
    }

    fclose(fp);
    fp = NULL;

    return true;
}

void getProcinfo(CpuContext *ctx)
{
    char ppath[BUFSIZ] = {0,};
    FILE *fp1 = NULL;
    static unsigned int total_proc_time;
    int cx = snprintf(ppath, sizeof(ppath), "/proc/%d/stat", ctx->_proc_t);
    if (!cx)
        return;

    fp1 = fopen( ppath, "r");
    if (fp1 == NULL)
    {
        fprintf(stderr, " '/proc/%d/stat' file open failed. \n", ctx->_proc_t);
        total_proc_time = 0;
        return;
    }

    char buf1[BUFSIZ] = {0,};

    unsigned long pUtime, pStime;
    unsigned long pCutime, pCstime;
    static unsigned long tproc_time, btproc_time;

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

    // irix mode (단일 코어를 기준 계산)
    // ctx->process_usage = CPU_EA * total_proc_time * 100 / ctx->system_total_usage;
    ctx->process_usage = total_proc_time * 100 / ctx->system_total_usage;

    fclose(fp1);
    
    return;
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