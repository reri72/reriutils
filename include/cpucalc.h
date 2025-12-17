#ifndef _CPUCALC_H_
#define _CPUCALC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define CLK_TCK sysconf(_SC_CLK_TCK)
#define CPU_EA sysconf(_SC_NPROCESSORS_ONLN)

extern unsigned long long total_cpu_usage;
extern unsigned long total_proc_time;
extern double pcpu_usage;
extern double cpu_usage;

void getSystemUptime();
void getSystemStat();
void getProcinfo(pid_t pid);
bool calCpu();

#endif  // _CPUCALC_H_