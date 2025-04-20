#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "reriutils.h"

typedef struct ifinfo
{
    char ipstr[INET6_ADDRSTRLEN];   // for ipv6
    unsigned int ipaddr;            // for ipv4
    char ifname[16];
    char ethernet[20];
    int isloop;
} ifinfo_t;

int main(int argc, char *argv[])
{
    struct ifaddrs *ifaddr = NULL, *ifa = NULL;
    if (get_interfaces(&ifaddr) == -1)
        return 0;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        ifinfo_t *_ifinfo = (ifinfo_t *)malloc(sizeof(ifinfo_t));
        memset(_ifinfo, 0, sizeof(ifinfo_t));

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;

            inet_ntop(AF_INET, &(sa->sin_addr), _ifinfo->ipstr, INET_ADDRSTRLEN);
            _ifinfo->ipaddr = ntohl(sa->sin_addr.s_addr);
            strncpy(_ifinfo->ifname, ifa->ifa_name, sizeof(_ifinfo->ifname) - 1);

            printf("ipv4 name : %s \n", ifa->ifa_name);
            printf("ipv4 addr : %s \n", _ifinfo->ipstr);
            printf("up : %d \n", ifa->ifa_flags & IFF_UP ? 1 : 0);
            printf("running : %d \n", ifa->ifa_flags & IFF_RUNNING ? 1 : 0);
            printf("loopback : %d \n", ifa->ifa_flags & IFF_LOOPBACK ? 1 : 0);
            printf("multicast : %d \n\n", ifa->ifa_flags & IFF_MULTICAST ? 1 : 0);
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ifa->ifa_addr;

            inet_ntop(AF_INET6, &(sa->sin6_addr), _ifinfo->ipstr, INET6_ADDRSTRLEN);
            strncpy(_ifinfo->ifname, ifa->ifa_name, sizeof(_ifinfo->ifname) - 1);

            printf("ipv6 name : %s \n", _ifinfo->ifname);
            printf("ipv6 addr : %s \n", _ifinfo->ipstr);
            printf("up : %d \n", ifa->ifa_flags & IFF_UP ? 1 : 0);
            printf("running : %d \n", ifa->ifa_flags & IFF_RUNNING ? 1 : 0);
            printf("loopback : %d \n", ifa->ifa_flags & IFF_LOOPBACK ? 1 : 0);
            printf("multicast : %d \n\n", ifa->ifa_flags & IFF_MULTICAST ? 1 : 0);
#if 0
            printf("full addr : ");
            for (int i = 0; i < 16; i++)
            {
                printf("%02x", sa->sin6_addr.s6_addr[i]);
                if ((i + 1) % 2 == 0 && i < 15)
                    printf(":");
            }
            printf("\n\n");
#endif
        }

        if (_ifinfo != NULL)
        {
            free(_ifinfo);
            _ifinfo = NULL;
        }
    }
    freeifaddrs(ifaddr);
        
    return 0;
}