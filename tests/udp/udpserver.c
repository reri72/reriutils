#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "socks.h"

int main(int argc, char **argv)
{
    int svrsockfd = -1;
    struct sockaddr_in client_addr;

    svrsockfd = create_sock(AF_INET, SOCK_DGRAM, 0);
    if (svrsockfd < 0)
    {
        return 1;
    }

    int ret = udp_server_process(svrsockfd, 1234, "10.0.2.6");
    if (ret == 0)
    {
        char buffer[65530] = {0,};
        char *message = "Hello client!";

        socklen_t client_len = 0;
        ssize_t recvlen = 0, sendlen = 0;

        client_len = sizeof(client_addr);

        recvlen = recvfrom(svrsockfd, buffer, sizeof(buffer), 0, 
                                (struct sockaddr*)&client_addr, &client_len);

        if (recvlen > 0)
            printf("recv : %s (%zd) \n", buffer, recvlen);

        sendlen = sendto(svrsockfd, message, strlen(message), 0, 
                            (struct sockaddr*)&client_addr, sizeof(client_addr));

        if (sendlen < 0)
            printf("sendto failed. (%zd) \n", sendlen);
    }
    else
    {
        printf("server socket failed \n");
    }
    
    close_sock(&svrsockfd);
    
    return 0;
}