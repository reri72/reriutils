#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "sockC.h"

#define PORT        5252
#define SERVER_IP   "10.0.2.5"

int main(int argc, char **argv)
{
#if HAVE_OPENSSL
    SSL_CTX *ctx = NULL;
    SSL     *ssl = NULL;

    int client_fd = -1;
    struct sockaddr_in server_addr;
    struct timeval timeout;

    init_ssl();

    ctx = create_ctx(0);
    if (ctx == NULL)
        goto EXIT_MAIN;

    ssl = SSL_new(ctx);
    if (ssl == NULL)
        goto EXIT_MAIN;

    client_fd = create_sock(AF_INET, SOCK_STREAM, 0);
    if (client_fd == FAILED)
    {
        perror("socket");
        goto EXIT_MAIN;
    }

    int ret = tcp_client_process(client_fd, PORT, SERVER_IP);
    if (ret != SUCCESS)
    {
        fprintf(stderr, "tcp_client_process");
        goto EXIT_MAIN;
    }

    if (SSL_set_fd(ssl, client_fd) == 0)
    {
        fprintf(stderr, "SSL_set_fd failed\n");
        goto EXIT_MAIN;
    }

    if (SSL_connect(ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
        goto EXIT_MAIN;
    }

    if (sock_set_nonblocking(client_fd) != SUCCESS)
    {
        fprintf(stderr, "sock_set_nonblocking");
        goto EXIT_MAIN;
    }

    while (1)
    {
        fd_set readfds;
        char buffer[1024] = {0,};
        
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(client_fd, &readfds);

        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        int activity = select(client_fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0)
        {
            perror("select error");
            break;
        }
        else if (activity == 0)
        {
            printf("timeout \n");
            continue;
        }

        if (FD_ISSET(client_fd, &readfds))
        {
            int bytes_read = read_ssl(ssl, buffer, sizeof(buffer) - 1);
            if (bytes_read < 1)
            {
                printf("error! bytes_read : %d \n", bytes_read);
                goto EXIT_MAIN;
            }
            printf("read success: %s (%d)\n", buffer, bytes_read);
        }

        break;
    }

EXIT_MAIN:

    cleanup_ssl(&ssl, &ctx);

    if (client_fd >= 0)
        close_sock(&client_fd);
#endif
    return 0;
}
