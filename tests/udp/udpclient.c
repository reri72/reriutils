#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "reriutils.h"

uint16_t port = 1234;
char ip[25] = "10.0.2.6";

int main(int argc, char **argv)
{
    int clisockfd = -1;

    char buffer[65530] = {0,};
    char *message = "Hello server!";
    struct sockaddr_in serv_addr;
    ssize_t sendlen = 0, recvlen = 0;

    clisockfd = create_sock(AF_INET, SOCK_DGRAM, 0);
    if (clisockfd < 0)
    {
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    
    sendlen = sendto(clisockfd, message, strlen(message), 0, 
            (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    if (sendlen < 0)
        printf("sendto failed (%zd) \n", sendlen);

    socklen_t serv_len = sizeof(serv_addr);
    recvlen = recvfrom(clisockfd, buffer, sizeof(buffer), 0, 
                            (struct sockaddr*)&serv_addr, &serv_len);

    if (recvlen > 0)
        printf("%s (%zd) \n", buffer, recvlen);

    close_sock(&clisockfd);

    return 0;
}