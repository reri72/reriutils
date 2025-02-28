#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <linux/if_ether.h>

#include "sockC.h"
#include "socks.h"

int create_sock(int domain, int type, int protocol)
{
    int sockfd = -1;

    sockfd = socket(domain, type, protocol);
    if (sockfd == -1)
    {
        return -1;
    }

    return sockfd;
}

int tcp_server_process(int sockfd, unsigned short port, char *svrip)
{
    int ret = FAILED;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(svrip); // INADDR_ANY;
    server_addr.sin_port = htons(port);

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == FAILED)
    {
        close_sock(&sockfd);
        return ret;
    }

    ret = listen(sockfd, BACKLOG);
    if (ret == FAILED)
    {
        perror("server sock listen failed \n");
        close_sock(&sockfd);
        return ret;
    }

    return SUCCESS;
}

int tcp_client_process(int sockfd, unsigned short port, char *svrip)
{
    int ret = FAILED;
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    ret = inet_pton(AF_INET, svrip, &serv_addr.sin_addr);
    if (ret != 1)
    {
        close_sock(&sockfd);
        return ret;
    }

    ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret != SUCCESS)
    {
        close_sock(&sockfd);
        return ret;
    }

    return SUCCESS;
}

int udp_server_process(int sockfd, unsigned short port, char *svrip)
{
    int ret = FAILED;
    
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(svrip); // INADDR_ANY;
    server_addr.sin_port = htons(port);

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == FAILED)
    {
        close_sock(&sockfd);
        return ret;
    }

    return SUCCESS;
}

void close_sock(int *sockfd)
{
    int ret = close(*sockfd);
    if (ret != 0)
    {
        perror("Failed to close the sockfd");
        return;
    }
    *sockfd = -1;
}
