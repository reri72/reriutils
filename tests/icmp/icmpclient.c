#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>

#include "sockC.h"

void sig_handle();
void *recvping(void *argv);
void *sendping(void *argv);

int sockfd = -1;
struct sockaddr_in icmpsend;

int sentcnt = 0;
int recvcnt = 0;

pid_t pid = 0;
pthread_t tid = -1;
int thr_id = -1;

int main(int argc, char **argv)
{
    sockfd = create_sock(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == FAILED)
    {
        printf("socket create failed \n");
        return 0;
    }

    signal(SIGINT, sig_handle);
    signal(SIGABRT, sig_handle);
    signal(SIGTERM, sig_handle);
    signal(SIGKILL, sig_handle);

    bzero((char *)&icmpsend, sizeof(icmpsend));

    icmpsend.sin_family = AF_INET;
    icmpsend.sin_addr.s_addr = inet_addr("8.8.8.8");

    pid = getpid() & 0xffff;

    thr_id = pthread_create(&tid, NULL, &recvping, NULL);
    if (thr_id != 0)
        abort();
    pthread_detach(tid);

    thr_id = pthread_create(&tid, NULL, &sendping, NULL);
    if (thr_id != 0)
        abort();
    pthread_detach(tid);

    while (sockfd > -1)
    {
        usleep(1000 * 100);
    }

    return 0;
}

void sig_handle()
{
    close_sock(&sockfd);
    printf("close socket.. \n");
    sleep(2);
}

void *recvping(void *argv)
{
    ssize_t recvlen = 0;
    char recvdata[256] = {0,};
    char fromip[INET_ADDRSTRLEN] = {0,};

    struct iphdr *iph = NULL;
    struct icmp *icmp = NULL;
    unsigned int iphdrlen = 0;
    unsigned int icmplen = 0;

    while (sockfd > -1)
    {
        recvlen = recvfrom(sockfd, recvdata, sizeof(recvdata), 0, NULL, NULL);
        if ( recvlen < 0)
        {
            printf("read failed (%zd) \n", recvlen);
            continue;
        }

        iph = (struct iphdr*)recvdata;
        iphdrlen = iph->ihl * 4;

        if (iph->protocol != IPPROTO_ICMP)
        {
            printf("not icmp packet \n");
            continue;
        }

        uint32_t ip = htonl(iph->saddr);
        if (inet_ntop(AF_INET, &ip, fromip, INET_ADDRSTRLEN) == NULL)
        {
            perror("inet_ntop");
            strncpy(fromip, "N/A", sizeof(fromip));
        }

        if (iph->saddr == icmpsend.sin_addr.s_addr) // check 8.8.8.8
        {
            icmp = (struct icmp *)(recvdata + iphdrlen);    // jump ip header
            icmplen = recvlen - iphdrlen;

            if (icmp->icmp_type == ICMP_ECHOREPLY)
            {
                if (icmp->icmp_id != pid)
                    continue;

                recvcnt++;
                printf("recv from %s  sent : %d  recv : %d \n", 
                                            fromip, sentcnt, recvcnt);
            }
        }
        recvdata[0] = 0;
    }
    close_sock(&sockfd);

    return NULL;
}

void *sendping(void *argv)
{
    char senddata[256] = {0,};
    while (sockfd > -1)
    {
        struct icmp *_icmp = NULL;
        
        char *data = "rucky reri";
        int sendlen = (sizeof(struct icmp) + strlen(data));

        _icmp = (struct icmp *)senddata;

        memset(_icmp->icmp_data, 0x00, strlen(data));

        _icmp->icmp_type = ICMP_ECHO;
        _icmp->icmp_code = 0;
        _icmp->icmp_cksum = 0;

        _icmp->icmp_id = pid;
        _icmp->icmp_seq = sentcnt++;

        memcpy(_icmp->icmp_data, data, strlen(data));

        _icmp->icmp_cksum = checksum((unsigned short *)_icmp, sendlen);

        ssize_t res = sendto(sockfd, senddata, sendlen, 0, (struct sockaddr *)&icmpsend, sendlen);

        printf("icmp send %s (%zd) \n", res > -1 ? "success" : "failed", res);

        senddata[0] = 0;
        sleep(1);
    }

    return NULL;
}