#ifndef _SOCKS_H_
#define _SOCKS_H_

#ifdef __cplusplus
extern "C" {
#endif

int create_sock(int domain, int type, int protocol);
int tcp_server_process(int sockfd, unsigned short port, char *svrip);
int tcp_client_process(int sockfd, unsigned short port, char *svrip);
int udp_server_process(int sockfd, unsigned short port, char *svrip);
void close_sock(int *sockfd);

#ifdef __cplusplus
}
#endif

#endif