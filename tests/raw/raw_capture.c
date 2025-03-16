#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>

#include <ifaddrs.h>
#include <netinet/if_ether.h>

#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "sockC.h"

bool stop = false;

void *ThreadRawCapture(void *arg);
void ParseNormalPacket(void *arg);

void sig_handle(int signo);

int main(int argc, char *argv[])
{
    if (check_rootuser() == -1)
        return 0;

    signal(SIGTERM, sig_handle);
    signal(SIGKILL, sig_handle);
    signal(SIGINT, sig_handle);
    signal(SIGILL, sig_handle);

    pthread_t pd;
    if ( pthread_create(&pd, NULL, ThreadRawCapture, NULL) == 0)
    {
        if (pthread_detach(pd) == 0)
            printf("thread detach!! \n");
    }

    struct timespec req, rem;
    req.tv_sec = 1;
    req.tv_nsec = 100000000;
    while (!stop)
        nanosleep(&req, &rem);

    sleep(1);
        
    return 0;
}

void *ThreadRawCapture(void *arg)
{
    struct sockaddr saddr;
    socklen_t saddr_len = sizeof(saddr);

    unsigned char buffer[BUFFER_SIZE] = {0,};
    int sockfd = -1;
    int res = -1;

    printf("[%s:%d] capture start\n", __FUNCTION__, __LINE__);

    sockfd = create_sock(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
    {
        perror("sock create failed \n");
        goto sockclose;
    }

    struct timeval timeout;
    fd_set readfds;
    int i = 0;
    while (!stop && i < 10)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        res = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (res && FD_ISSET(sockfd, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));

            // MSG_PEEK : check buffer message
            ssize_t recvsize = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, &saddr, &saddr_len);
            if (recvsize < 0 || recvsize < 14)
            {
                perror("recv failed");
                goto sockclose;
            }

            printf("Packet received: %ld bytes\n", recvsize);

            ParseNormalPacket(buffer);

            printf("Packet Content (Raw):\n");
            for (int i = 0; i < recvsize; i++) {
                printf("%02x ", (unsigned char)buffer[i]);
                if ((i + 1) % 16 == 0)
                    printf("\n");
            }
            printf("\n\n");
        }
        i++;
    }
    
sockclose :
    stop = true;
    close_sock(&sockfd);

    return NULL;
}

void ParseNormalPacket(void *arg)
{
    char *packet = (char *)arg;
    uint16_t proto = -1;
    uint8_t type = -1;

    proto = parse_etherframe(packet);
    if (proto == ETH_P_IP)
    {
        type = parse_ipheader(packet);
    }
    
    if (type == IPPROTO_TCP)
    {
        parse_tcpheader(packet);
    } 
    else if (type == IPPROTO_UDP)
    {
        parse_udpheader(packet);
    }
}

void sig_handle(int signo)
{
    printf("signal : %d \n", signo);
    stop = true;
}
