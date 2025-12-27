#ifndef _DISKCALC_H_
#define _DISKCALC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>

/*  definition  */
#define KB 1024
#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mountinfo 
{
    FILE *fp;
    char devname[100];
    char mountdir[100];
    char fstype[20];
    unsigned long blocks;
    unsigned long avail;
    double usage_percent;
}  __attribute__((packed)) Mountinfo;

/*  functions    */
void GetDiskUsage_Loop();
double CalculateDiskUsage(char *dir);
Mountinfo *MountOpen();
Mountinfo *ReadMountInfo(Mountinfo *minfo);
void MountReadClose(Mountinfo *minfo);

#ifdef __cplusplus
}
#endif

#endif  // _DISKCALC_H_
