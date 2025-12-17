#include "diskcalc.h"

struct statvfs diskusage;
Mountinfo *minfo;

void GetDiskUsage_Loop()
{
    Mountinfo *minfo_ptr = MountOpen();
    if (!minfo_ptr)
    {
        fprintf(stderr, "Failed to open /proc/mounts.\n");
        return;
    }

    while (ReadMountInfo(minfo_ptr))
    {
        Once_MountInfo(minfo_ptr->mountdir);
    }

    MountReadClose(minfo_ptr);
}

Mountinfo *MountOpen()
{
    Mountinfo *mountinfo = (Mountinfo*)malloc( sizeof(Mountinfo) );
    memset(mountinfo, 0x00, sizeof(Mountinfo));

    if (!(mountinfo->fp = fopen("/proc/mounts", "r")))
    {
        free(mountinfo);
        return NULL;
    }

    return mountinfo;
}

Mountinfo *ReadMountInfo(Mountinfo *minfo)
{
    char buf[1024] = {0,};
    int rootdir = 0;

    struct statfs lstatfs;
    struct stat lstat;

    memset(&lstatfs, 0x00, sizeof(struct statfs));
    memset(&lstat, 0x00, sizeof(struct stat));

    // /proc/mounts 의 행을 모두 읽는다
    while( fgets(buf, 1024, minfo->fp) )
    {
        sscanf(buf, "%128s %128s %128s", minfo->devname, minfo->mountdir, minfo->fstype);
        
        if ( strcmp(minfo->mountdir, "/") == 0)
        {
            rootdir = 1;
        }
        else
        {
            rootdir = 0;
        }

        // stat : get file status
        if (stat(minfo->devname, &lstat) == 0 || rootdir)
        {
            // block device(storage device) 확인한다.
            if (( S_ISBLK(lstat.st_mode) && strstr(buf, minfo->mountdir) ) || rootdir)
            {
                statfs(minfo->mountdir, &lstatfs);

                minfo->blocks = lstatfs.f_blocks * (lstatfs.f_bsize / KB); 
                minfo->avail = lstatfs.f_bavail * (lstatfs.f_bsize / KB); 

                return minfo;
            }
        }
    }

    return NULL;
}

void MountReadClose(Mountinfo *minfo)
{
    fclose(minfo->fp);
    if (minfo)
    {
        free(minfo);
    }
}

double Once_MountInfo(char *dir)
{
    unsigned long long total = 0, used = 0;
    double usage_percentage = 0;

    if (statvfs(dir, &diskusage) != 0 )
    {
        fprintf(stderr, "statvfs error: %s\n", dir);
        return 0.0;
    }

    // f_blocks : 파일 시스템의 전체 블록 수
    // f_frsize : 블록 크기
    total = (unsigned long long)diskusage.f_blocks * diskusage.f_frsize;

    // f_bfree : 슈퍼유저를 포함한 사용 가능한 블록 수
    // f_bavail : 일반 사용자에게 사용 가능한 블록 수
    // df와 동일한 결과를 얻으려면 f_blocks - f_bfree 를 사용
    used = (unsigned long long)(diskusage.f_blocks - diskusage.f_bfree) * diskusage.f_frsize;

    if (total > 0)
    {
        usage_percentage = ((double)used / (double)total) * 100.0;
    }
    else
    {
        usage_percentage = 0.0;
    }

    // printf("Linux disk usage(%%) - [%s] : %0.2f %% \n", dir, usage_percentage);

    return usage_percentage;
}
