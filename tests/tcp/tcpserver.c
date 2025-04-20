#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "reriutils.h"

int main(int argc, char **argv)
{
    int svrsockfd = -1, clisockfd = -1;

    svrsockfd = create_sock(AF_INET, SOCK_STREAM, 0);
    if (svrsockfd < 0)
    {
        return 1;
    }

    int ret = tcp_server_process(svrsockfd, 1234, "10.0.0.3");
    if (ret == 0)
    {
        clisockfd = accept(svrsockfd, NULL, NULL);
        if (clisockfd == -1)
        {
            perror("server sock accept failed \n");
            close_sock(&svrsockfd);
            return -1;
        }

        printf("accept success!! \n");

        char *message = "Hello, Client!";
        char buffer[1024] = {0,};

        send(clisockfd, message, strlen(message), 0);
        printf("send to client %s\n", message);

        int valread = recv(clisockfd, buffer, 1024, 0);
        if (valread < 0)
        {
            perror("recv failed !!");
        }
        else
        {
            printf("recv from client: %s\n", buffer);
        }

        close_sock(&clisockfd);
        close_sock(&svrsockfd);
    }
    else
    {
        printf("server socket failed \n");
    }
    
    return 0;
}