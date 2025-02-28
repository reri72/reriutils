#ifndef _SOCKUTILES_H_
#define _SOCKUTILES_H_

#include <ifaddrs.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pseudoheader{
    uint32_t src;
    uint32_t dest;
    uint8_t reserved;
    uint8_t protocol;
    uint16_t len;
} pseudoheader_t;

int sock_set_linger(int sockfd, int sec);
int sock_set_buffer_size(int sockfd, int size);
int sock_set_keep_alive(int sockfd, int idle_sec, int intvl_cnt, int retry_cnt);
int sock_set_no_delay(int sockfd);
int sock_set_reuse(int sockfd);

int sock_set_nonblocking(int sockfd);

unsigned short parse_etherframe(void *packet);
uint8_t parse_ipheader(void *packet);
void parse_tcpheader(void *packet);
void parse_udpheader(void *packet);

unsigned short checksum(unsigned short *addr, int len);

int get_interfaces(struct ifaddrs **ifaddr);
void get_etherstring(unsigned char *input, char *output, size_t len);

#ifdef __cplusplus
}
#endif

#endif