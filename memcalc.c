#include "memcalc.h"

bool readMemInfo(Meminfo *mems)
{
    FILE * fp = fopen( "/proc/meminfo", "r");

    if (!fp)
        return false;

    char line[256] = {0,};
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "MemTotal:", 9) == 0)
            sscanf(line, "%*s %lu", &mems->mtot);
        else if (strncmp(line, "MemFree:", 8) == 0)
            sscanf(line, "%*s %lu", &mems->fmree);
        else if (strncmp(line, "MemAvailable:", 13) == 0)
            sscanf(line, "%*s %lu", &mems->mavail);
        else if (strncmp(line, "Buffers:", 8) == 0)
            sscanf(line, "%*s %lu", &mems->bufs);
        else if (strncmp(line, "Cached:", 7) == 0)
            sscanf(line, "%*s %lu", &mems->cached);
        else if (strncmp(line, "SwapCached:", 11) == 0)
            sscanf(line, "%*s %lu", &mems->swapcached);
    }

    fclose(fp);

    return true;
}

bool readProcMemInfo(pMeminfo *pmems, pid_t pid)
{
    char ppath[100] = {0,};
    int cx = 0;
    FILE *fp = NULL;

    cx = snprintf( ppath, sizeof(ppath), "/proc/%d/status", pid);
    if (!cx)
        return false;

    fp = fopen(ppath, "r");
    if (fp == NULL)
        return false;
    
    memset(pmems, 0x00, sizeof(pMeminfo));

    int i = 0;
    unsigned long val[4] = {0, };    

    char* list[4] = {"VmPeak:", "VmSize:", "VmData:", "VmRSS:"};
    char temp[1024] = {0,};
    unsigned long int val_temp = 0;
    char item[64] = {0,};
    char *pstr = NULL;
    int k;

    while (fgets(temp, sizeof(temp), fp) && i < 4)
    {
        if (sscanf(temp, "%63s %lu", item, &val_temp) == 2)
        {
            for (k = 0; k < 4; k++)
            {
                if (strcasecmp(item, list[k]) == 0)
                {
                    if (k == 0)
                        pmems->VmPeak = val_temp;
                    else if (k == 1)
                        pmems->VmSize = val_temp;
                    else if (k == 2)
                        pmems->VmData = val_temp;
                    else if (k == 3)
                        pmems->VmRSS = val_temp;

                    i++;
                    break;
                }
            }
        }
    }

    fclose(fp);

    return true;
}

bool ReadProcStatm(pStatm *pstatm, pid_t pid)
{
    char path[100] = {0,};
    int cx = 0;

    memset(pstatm, 0x00, sizeof(pStatm));

    cx = snprintf( path, sizeof(path), "/proc/%d/statm", pid);
    if (!cx)
    {
        return false;
    }

    FILE * fp = fopen(path, "r");
    if (fp != NULL)
    {
        char buf[1024] = {0,};
        fgets(buf, 1024, fp);

        sscanf(buf, "%lu %lu %lu %lu %lu %lu %lu",
            &pstatm->size, &pstatm->resident, &pstatm->share, &pstatm->text, &pstatm->lib, &pstatm->data, &pstatm->dt);
    
        fclose(fp);
    }
    else
    {
        return false;
    }

    return true;
}