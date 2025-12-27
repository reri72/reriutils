#include "diskcalc.h"

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
        CalculateDiskUsage(minfo_ptr->mountdir);
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
            if (S_ISBLK(lstat.st_mode) || rootdir)
            {
                if (minfo->fstype[0] == 'n' && minfo->fstype[1] == 'f')
                    continue; // nfs(네트워크파일시스템) 제외
                if (strcmp(minfo->fstype, "tmpfs") == 0)
                    continue; // tmpfs(임시파일시스템) 제외
                    
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

double CalculateDiskUsage(char *dir)
{
    struct statvfs vfs;
    if (statvfs(dir, &vfs) != 0)
        return 0.0;

    // f_blocks : 파일 시스템의 전체 블록 수
    // f_frsize : 블록 크기
    unsigned long long total = (unsigned long long)vfs.f_blocks * vfs.f_frsize;

    // f_bfree : 슈퍼유저를 포함한 사용 가능한 블록 수
    // f_bavail : 일반 사용자에게 사용 가능한 블록 수
    // df와 동일한 결과를 얻으려면 f_blocks - f_bfree 를 사용
    unsigned long long used = (unsigned long long)(vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize;
    unsigned long long free = (unsigned long long)vfs.f_bavail * vfs.f_frsize;
    
    if (total == 0)
        return 0.0;
    
    return (double)(total - free) / total * 100.0;
}
