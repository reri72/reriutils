#include "memcalc.h"

/*  variables   */
Meminfo mems;
pMeminfo pmems;
pStatm pstatm;

double real_mem;
double pretext_mem;
double pmem;

bool readMemInfo(Meminfo *mems)
{
    FILE * fp = fopen( "/proc/meminfo", "r");

    int i = 0;
    unsigned long val[6] = {0, };

    memset(mems, 0x00, sizeof(Meminfo));

    if (fp != NULL)
    {
        char temp[1024] = {0,};
        char *pstr = NULL;

        do
        {
            // fgets는 개행('\n')이 있는 곳 까지(줄바꿈 등장까지) 읽음
            // 구조체에 필요한 6줄만 읽는다.
            pstr = fgets(temp, sizeof(temp), fp);
            sscanf(temp, "%*s %lu %*s", &val[i]);
            i++;

        } while(i < 6);

        fclose(fp);
    }
    else
    {
        return false;
    }

    mems->mtot = val[0];
    mems->fmree = val[1];
    mems->mavail = val[2];
    mems->bufs = val[3];
    mems->cached = val[4];
    mems->swapcached = val[5];

    return true;
}

bool readProcMemInfo(pMeminfo *pmems, pid_t pid)
{
    char ppath[100] = {0,};
    int cx = 0;

    memset(pmems, 0x00, sizeof(pMeminfo));

    cx = snprintf( ppath, sizeof(ppath), "/proc/%d/status", pid);
    if (!cx)
    {
        return false;
    }

    FILE * fp = fopen(ppath, "r");

    int i = 0;
    unsigned long val[4] = {0, };    

    if (fp != NULL)
	{
        char* list[4] = {"VmPeak:", "VmSize:", "VmData:", "VmRSS:"};
		char temp[1024] = {0,};
        char item[64] = {0,};
		char *pstr = NULL;
        int k;

		do
		{
			pstr = fgets(temp, sizeof(temp), fp);
            sscanf(temp, "%63s %lu", item, &val[i]);
            for (k = 0; k < 4; k++)
            {
                if (strcasecmp(item, list[k]) == 0)
                {
                    i++;
                    break;
                }
            }
            
		}while(i < 4);

		fclose(fp);
	}
	else
	{
        return false;
	}

    pmems->VmPeak = val[0];
    pmems->VmSize = val[1];
    pmems->VmData = val[2];
    pmems->VmRSS = val[3];

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

        sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld",
            &pstatm->size, &pstatm->resident, &pstatm->share, &pstatm->text, &pstatm->lib, &pstatm->data, &pstatm->dt);
    
        fclose(fp);
    }
    else
    {
        return false;
    }

    return true;
}