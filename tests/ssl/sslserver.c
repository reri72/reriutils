#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "sockC.h"
#include "socks.h"
#include "sslUtils.h"

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
    if (server_fd < 0)
    {
        perror("socket");
        goto MAIN_EXIT;
    }

    int ret = tcp_server_process(server_fd, PORT, SERVER_IP);
    if (ret == 0)
    {
        printf("Waiting for a client on %s:%d...\n", SERVER_IP, PORT);
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
        else
        {
            printf("client accept! \n");

            char buffer[1024] = {0};
            int reads = SSL_read(ssl, buffer, sizeof(buffer));
            if (reads > 0)
            {
                buffer[reads] = '\0';
                printf("server read : %s(%d) \n", buffer, reads);
            }
            else
            {
                err = SSL_get_error(ssl, reads);
                check_ssl_err(err);

                if (err == SSL_ERROR_ZERO_RETURN)
                    SSL_shutdown(ssl);

                goto MAIN_EXIT;
            }

            char reply[256] = "Hello, SSL Client!";
            int writes = SSL_write(ssl, reply, strlen(reply));
            if (writes > 0)
            {
                printf("server write : %s(%d)\n", reply, writes);
            }
            else
            {
                err = SSL_get_error(ssl, writes);
                check_ssl_err(err);

                if (err == SSL_ERROR_ZERO_RETURN)
                    SSL_shutdown(ssl);
                goto MAIN_EXIT;
            }
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

