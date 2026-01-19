#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "reriutils.h"

#define PORT        5252
#define CERT_FILE   "cert.crt"
#define KEY_FILE    "cert.key"

#define SERVER_IP   "10.0.2.5"

int main(int argc, char **argv)
{
#if HAVE_OPENSSL
    SSL_CTX *ctx = NULL;
    SSL     *ssl = NULL;
    
    int server_fd = -1, client_fd = -1;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    init_ssl();
    ctx = create_ctx(1);
    if (ctx == NULL)
        goto MAIN_EXIT;

    if (configure_ctx(ctx, CERT_FILE, KEY_FILE) < 0)
        goto MAIN_EXIT;

    server_fd = create_sock(AF_INET, SOCK_STREAM, 0);
    if (server_fd == FAILED)
    {
        perror("socket");
        goto MAIN_EXIT;
    }

    if (sock_set_nonblocking(server_fd) != SUCCESS)
        goto MAIN_EXIT;

    int ret = tcp_server_process(server_fd, PORT, SERVER_IP);
    if (ret != SUCCESS)
        goto MAIN_EXIT;

    while (1)
    {
        fd_set readfds;
        struct timeval timeout;
        char buffer[1024] = {0,};

        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0)
        {
            // 시스템 신호에 의해 중단된 경우 다시 시도
            if (errno == EINTR)
                continue;
            perror("select error");
            break;
        }
        else if (activity == 0)
        {
            printf("timeout \n");
            continue;
        }

        printf("Waiting for a client on %s:%d...\n", SERVER_IP, PORT);
        if (FD_ISSET(server_fd, &readfds))
        {
            client_fd = accept(server_fd, (struct sockaddr*)&addr, &addr_len);
            if (client_fd < 0)
            {
                perror("accept");
                goto MAIN_EXIT;
            }

            ssl = SSL_new(ctx);
            if (ssl == NULL)
                goto MAIN_EXIT;

            if (SSL_set_fd(ssl, client_fd) == 0)
            {
                fprintf(stderr, "SSL_set_fd failed\n");
                goto MAIN_EXIT;
            }

            int err = SSL_ERROR_ZERO_RETURN;
            if (SSL_accept(ssl) <= 0)
            {
                int ssl_err = SSL_get_error(ssl, -1);
                check_ssl_err(ssl_err);
                ERR_print_errors_fp(stderr);
            }
            
            int bytes_write = write_ssl(ssl, "hello client ?", strlen("hello client?"));
            if (bytes_write < 1)
            {
                printf("error ! bytes_write : %d \n", bytes_write);
                goto MAIN_EXIT;
            }
            printf("send success \"hello client ?\" (%d)\n", bytes_write);

            break;
        }
    }

MAIN_EXIT:

    cleanup_ssl(&ssl, &ctx);

    if (client_fd >= 0)
        close_sock(&client_fd);

    if (server_fd >= 0)
        close_sock(&server_fd);

#endif
}

