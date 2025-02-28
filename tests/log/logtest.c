#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "writelog.h"

int main(int argc, char const *argv[])
{
    char pwd[MAX_LOG_FULLPATH_SIZE] = {0,};
    if (getcwd(pwd, sizeof(pwd)) != NULL)
    {
        printf("Current working dir: %s \n", pwd);
    }
    else
    {
        perror("getcwd() error!!");
        exit(0);
    }

    init_log(LOG_DEBUG, 4096);
    create_logfile(pwd,"testlog.log");
    
    int i = 0;
    while (i < 50)
    {
        LOG_INFO("hello~ \n");
        LOG_DEBUG("hi ~ \n");
        LOG_ERR("bye~ \n");
        LOG_WARN("bi \n");
        i++;
    }

    // --------------------------------------------------------------

    unsigned char str[64] = "hello world! bye world! good morning~";
    size_t len = strlen(str);

    PACKETDUMP(str, len);
    
    LOG_WARN("bi \n");

    sleep(1);
    destroy_log();
    
    return 0;
}
