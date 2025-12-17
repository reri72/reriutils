#ifndef _MEMCALC_H_
#define _MEMCALC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*	https://github.com/reri72	*/

/*  definition  */
#define PAGESIZE getpagesize()
#define PAGE_COUNT PAGESIZE / 1024

typedef struct meminfo
{
    unsigned long mtot;
    unsigned long fmree;
    unsigned long mavail;
    unsigned long bufs;
    unsigned long cached;
    unsigned long swapcached;
} __attribute__((packed)) Meminfo;

typedef struct pmeminfo
{   // check on linux. ( ]# ps -aux )
    unsigned long VmPeak;
    unsigned long VmSize;   //real mem usage KB. (= statm.size)
    unsigned long VmData;
    unsigned long VmRSS;
} __attribute__((packed)) pMeminfo;

// /proc/PID/statm
// 페이지 단위로 측정된 비트
typedef struct statm
{
    unsigned long size;
    unsigned long resident;
    unsigned long share;
    unsigned long text;
    unsigned long lib;
    unsigned long data;
    unsigned long dt;
}__attribute__((packed)) pStatm;


/*  variables   */
extern Meminfo mems;
extern pMeminfo pmems;
extern pStatm pstatm;

extern double real_mem;
extern double pretext_mem;
extern double pmem;


/*  functions    */
bool readMemInfo(Meminfo *mems);
bool readProcMemInfo(pMeminfo *pmems, pid_t pid);
bool ReadProcStatm(pStatm *pstatm, pid_t pid);

#endif //_MEMCALC_H_