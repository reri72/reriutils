#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "sockC.h"
#include "sslUtils.h"
#include "socks.h"

#define PORT        5252
#define SERVER_IP   "10.0.2.5"

int main(int argc, char **argv)
{
#if HAVE_OPENSSL
    SSL_CTX *ctx = NULL;
    SSL     *ssl = NULL;

    int client_fd = -1;
    struct sockaddr_in server_addr;

    init_ssl();

    ctx = create_ctx(0);
    if (ctx == NULL)
        goto EXIT_MAIN;

    ssl = SSL_new(ctx);
    if (ssl == NULL)
        goto EXIT_MAIN;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("socket");
        goto EXIT_MAIN;
    }

    int ret = tcp_client_process(client_fd, PORT, SERVER_IP);
    if (ret == 0)
    {
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

        int err = SSL_ERROR_ZERO_RETURN;
        char sendmsg[256] = "Hello, SSL Server!";
        int writes = SSL_write(ssl, sendmsg, strlen(sendmsg));
        if (writes > 0)
        {
            printf("Client write : %s(%d)\n", sendmsg, writes);
        }
        else
        {
            err = SSL_get_error(ssl, writes);
            check_ssl_err(err);

            if (err == SSL_ERROR_ZERO_RETURN)
                SSL_shutdown(ssl);

            goto EXIT_MAIN;
        }

        char readmsg[256] = {0};
        int reads = SSL_read(ssl, readmsg, sizeof(readmsg));
        if (reads > 0)
        {
            printf("Server: %s(%d)\n", readmsg, reads);
        }
        else
        {
            err = SSL_get_error(ssl, reads);
            check_ssl_err(err);

            if (err == SSL_ERROR_ZERO_RETURN)
                SSL_shutdown(ssl);

            goto EXIT_MAIN;
        }
    }

EXIT_MAIN:

    cleanup_ssl(&ssl, &ctx);

    if (client_fd >= 0)
        close_sock(&client_fd);
#endif
    return 0;
}
