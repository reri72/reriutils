#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>

#include "reriutils.h"

int sock_set_linger(int sockfd, int sec)
{
    int ret = FAILED;
    struct linger linger_opt;

    linger_opt.l_onoff  = 1;
    linger_opt.l_linger = sec;

    ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
    if (ret != SUCCESS)
    {
        perror("SO_LINGER failed.");
        return FAILED;
    }

    return SUCCESS;
}

int sock_set_buffer_size(int sockfd, int size)
{
    int ret = FAILED;
    int bufsize = size;

    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    if (ret != SUCCESS)
    {
        perror("Sendbuffer size option failed.");
        return FAILED;
    }

    ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    if (ret != SUCCESS)
    {
        perror("Recv buffer size option failed.");
        return FAILED;
    }

    return SUCCESS;
}

int sock_set_keep_alive(int sockfd, int idle_sec, int intvl_cnt, int retry_cnt)
{
    int ret = FAILED;
    int opt = 1;

    ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
    if (ret != SUCCESS)
    {
        perror("SO_KEEPALIVE failed.");
        return FAILED;
    }

    if (idle_sec)
    {
        // TCP_KEEPIDLE: keep alive 전송까지 대기 시간 (초 단위)
        ret = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &idle_sec, sizeof(idle_sec));
        if (ret != SUCCESS)
        {
            perror("TCP_KEEPIDLE failed");
        }
    }

    if (intvl_cnt)
    {
         // TCP_KEEPINTVL: keep alive  재전송 간격 (초 단위)
        ret = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl_cnt, sizeof(intvl_cnt));
        if (ret != SUCCESS)
        {
            perror("TCP_KEEPINTVL failed");
        }
    }

    if (retry_cnt)
    {
        // TCP_KEEPCNT: keep alive 재전송 횟수
        ret = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &retry_cnt, sizeof(retry_cnt));
        if (ret != 0)
        {
            perror("Setting TCP_KEEPCNT failed");
        }
    }

    return SUCCESS;
}

int sock_set_no_delay(int sockfd)
{
    int ret = FAILED;
    int opt = 1;

    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    if (ret != SUCCESS)
    {
        perror("NoDelay option failed.");
        return FAILED;
    }

    return SUCCESS;
}

int sock_set_reuse(int sockfd)
{
    int ret = FAILED;
    int opt = 1;

    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret != 0)
    {
        perror("SO_REUSEADDR failed");
        return FAILED;
    }

#ifdef SO_REUSEPORT
    // SO_REUSEPORT 옵션이 지원되는 시스템에서만 사용 가능하다.
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    if (ret != 0)
    {
        perror("SO_REUSEPORT failed");
        return FAILED;
    }
#endif

    return SUCCESS;
}

int sock_set_nonblocking(int sockfd)
{
    int ret = FAILED;
    int flags = -1;

    flags = fcntl(sockfd, F_GETFL, 0);      //get file flag
    if (flags == -1)
    {
        perror("fcntl(F_GETFL) failed.");
        return ret;
    }

    flags |= O_NONBLOCK;

    ret = fcntl(sockfd, F_SETFL, flags);    // set file flag
    if (ret == -1)
    {
        perror("fcntl(F_SETFL) failed.");
        return ret;
    }
    
    return SUCCESS;
}

unsigned short parse_etherframe(void *packet)
{
    struct ethhdr *ethdr = (struct ethhdr *)packet;
    char destethr[20] = {0,}, srcethr[20] = {0,};

    get_etherstring(ethdr->h_source, srcethr, sizeof(srcethr));
    get_etherstring(ethdr->h_dest, destethr, sizeof(destethr));

    printf("ether src : %s \n", srcethr);
    printf("ether dst : %s \n", destethr);
    printf("ether proto : %u \n", ntohs(ethdr->h_proto));

    return (ntohs(ethdr->h_proto));
}

uint8_t parse_ipheader(void *packet)
{
    struct iphdr *ip_header = (struct iphdr *)(packet + sizeof(struct ethhdr));
    struct sockaddr_in source, dest;
    uint16_t orig = 0, verify = 0;
    char destethr[18] = {0,}, srcethr[18] = {0,};

    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip_header->saddr;

    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip_header->daddr;

    printf("ip src : %s\n", inet_ntoa(source.sin_addr));
    printf("ip dst: %s\n", inet_ntoa(dest.sin_addr));
    printf("ip proto: %d\n", ip_header->protocol);

    orig = ip_header->check;

    ip_header->check = 0;
    verify = checksum((unsigned short *)ip_header, ip_header->ihl*4);
    if (orig != verify)
    {
        // do somethigs
    }

    ip_header->check = orig;
    
    return (ip_header->protocol);
}

void parse_tcpheader(void *packet)
{
    struct iphdr *ipdata = (struct iphdr *)(packet + sizeof(struct ethhdr));
    struct tcphdr *tcpdata = (struct tcphdr *)(packet + sizeof(struct ethhdr) + (ipdata->ihl * 4));
    pseudoheader_t pheader;

    unsigned char *chkbuffer = NULL;
    uint16_t sum = 0;
    uint16_t org = tcpdata->th_sum;

    memset(&pheader, 0, sizeof(pseudoheader_t));

    tcpdata->th_sum = 0;
    pheader.src = ipdata->saddr;
    pheader.dest = ipdata->daddr;
    pheader.reserved = 0;
    pheader.protocol = IPPROTO_TCP;
    // IP 헤더 크기를 제외한 전체 길이 설정
    pheader.len = htons(ntohs(ipdata->tot_len) - (ipdata->ihl * 4));

    chkbuffer = (unsigned char *)malloc( (sizeof(pseudoheader_t) + ntohs(pheader.len)) );
    memset(chkbuffer, 0, (sizeof(pseudoheader_t) + ntohs(pheader.len)));
    if (chkbuffer != NULL)
    {
        memcpy(chkbuffer, (unsigned char *)&pheader, sizeof(pseudoheader_t));
        memcpy(chkbuffer + sizeof(pseudoheader_t), (unsigned char *)tcpdata, ntohs(pheader.len));

        sum = checksum((unsigned short *)chkbuffer, (sizeof(pseudoheader_t) + ntohs(pheader.len)));
        if (org != sum)
        {
            // do somethings
        }
        free(chkbuffer);
    }
    
    tcpdata->th_sum = org;
}

void parse_udpheader(void *packet)
{
    struct iphdr *ipdata = (struct iphdr *)(packet + sizeof(struct ethhdr));
    struct udphdr *udpdata = (struct udphdr *)(packet + sizeof(struct ethhdr) + (ipdata->ihl * 4));
    uint16_t org = udpdata->check;
    
    unsigned char *chkbuffer = NULL;
    pseudoheader_t pheader;

    memset(&pheader, 0, sizeof(pseudoheader_t));
    
    printf("udp src : %u \n", ntohs(udpdata->source));
    printf("udp dst : %u \n", ntohs(udpdata->dest));
    printf("udp len : %u \n", ntohs(udpdata->len));
    printf("udp checksum : %u \n", ntohs(udpdata->check));

    udpdata->check = 0;

    pheader.src = ipdata->saddr;
    pheader.dest = ipdata->daddr;
    pheader.reserved = 0;
    pheader.protocol = IPPROTO_UDP;
    pheader.len = udpdata->len;
    
    chkbuffer = (unsigned char *)malloc( (sizeof(pseudoheader_t) + ntohs(pheader.len)) );
    memset(chkbuffer, 0, (sizeof(pseudoheader_t) + ntohs(pheader.len)));
    if (chkbuffer != NULL)
    {
        memcpy(chkbuffer, (unsigned char *)&pheader, sizeof(pseudoheader_t));
        memcpy(chkbuffer + sizeof(pseudoheader_t), (unsigned char *)udpdata, ntohs(pheader.len));

        uint16_t chk = checksum((unsigned short *)chkbuffer, (sizeof(pseudoheader_t) + ntohs(pheader.len)));
        if (org != chk)
        {
            // do somethings
        }
        free(chkbuffer);
    }

    udpdata->check = org;
}

unsigned short checksum(unsigned short *addr, int len)
{
    int datalen = len;
    int sum = 0;

    unsigned short *p = addr;
    unsigned short res = 0;

    while (datalen > 1) 
    {
        sum += *p++;
        datalen -= (sizeof(unsigned short));     //reduce 2 bytes
    }

    if (datalen == 1) 
    {
        *(unsigned short *)(&res) = *(unsigned short *)p;
        sum += res;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);                     //carry
    res = ~sum;

    return res;
}

int get_interfaces(struct ifaddrs **ifaddr)
{
    // get interfaces
    if (getifaddrs(ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    // have to use 'freeifaddrs' function after.
    
    return 0;
}

void get_etherstring(unsigned char *input, char *output, size_t len)
{
    uint64_t tmp = 0;
    memcpy(&tmp, input, sizeof(uint8_t) * 6);

    if (tmp == 0)
    {
        snprintf(output, len, "00:00:00:00:00:00");
    }
    else
    {
        snprintf(output, len, 
            "%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char)input[0], (unsigned char)input[1], 
            (unsigned char)input[2], (unsigned char)input[3], 
            (unsigned char)input[4], (unsigned char)input[5]);
    }
}
