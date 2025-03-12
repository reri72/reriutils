#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "utiles.h"

void nano_sleep(int sec, int nsec)
{
    struct timespec rem, req;

    req.tv_sec = sec;
    req.tv_nsec = nsec;

    while (nanosleep(&req, &rem) == -1)
    {
        if (errno == EINTR)
        {
            req = rem;
        }
        else
        {
            perror("nanosleep failed");
            break;
        }
    }

    return;
}

int check_rootuser()
{
    if (geteuid() == 0)
    {
        return 0;
    }
    return -1;
}

// success : 0 , failed : -1
int check_exist_file(const char *fullpath)
{
	int res = access(fullpath, 0);
    
	return res;
}