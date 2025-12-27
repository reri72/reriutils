#ifndef _CPUCALC_H_
#define _CPUCALC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define CLK_TCK sysconf(_SC_CLK_TCK)
#define CPU_EA sysconf(_SC_NPROCESSORS_ONLN)

typedef struct {
    double system_total_usage;
    double process_usage;
    unsigned long long _prev_total;
    unsigned long long _prev_idle;
    unsigned long _proc_t;
} CpuContext;

bool update_cpu_usage(CpuContext *ctx);
void getProcinfo(CpuContext *ctx);

void getSystemUptime();


#ifdef __cplusplus
}
#endif

#endif  // _CPUCALC_H_