#include "sslUtils.h"

#if HAVE_OPENSSL

int init_ssl()
{
    if (OPENSSL_init_ssl(0, NULL) == 0)
    {
        fprintf(stderr, "failed to initialize SSL \n");
        return 1;
    }
    return 0;
}

void cleanup_ssl(SSL **ssl, SSL_CTX **ctx)
{
    if (*ssl != NULL)
    {
        SSL_free(*ssl);
        *ssl = NULL;
    }

    if (*ctx != NULL)
    {
        SSL_CTX_free(*ctx);
        *ctx = NULL;
    }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_cleanup();  //암호화 알고리즘 관련 리소리 정리
#endif
}

void check_ssl_err(int err)
{
    switch (err)
    {
        case SSL_ERROR_ZERO_RETURN:
            printf("SSL_ERROR_ZERO_RETURN \n");
            break;
        case SSL_ERROR_SSL:
            printf("SSL_ERROR_SSL \n");
            ERR_print_errors_fp(stderr);
            break;
        case SSL_ERROR_WANT_READ:
            printf("SSL_ERROR_WANT_READ \n");
            break;
        case SSL_ERROR_WANT_WRITE:
            printf("SSL_ERROR_WANT_WRITE \n");
            break;
        case SSL_ERROR_SYSCALL:
            printf("SSL_ERROR_SYSCALL \n");
            perror("SSL syscall error");
            break;
        default:
            printf("SSL_read() failed : %d\n", err);
            ERR_print_errors_fp(stderr);
            break;
    }
}

SSL_CTX *create_ctx(int svr)
{
    SSL_CTX *ctx = NULL;
    
    if (svr)
        ctx = SSL_CTX_new(TLS_server_method());
    else
        ctx = SSL_CTX_new(TLS_client_method());

    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);    // 발생 오류를 stderr 에 출력
        return NULL;
    }
    return ctx;
}

int configure_ctx(SSL_CTX *ctx, const char *certpath, const char *keypath)
{
    // SSL_CTX 구조체에 인증서를 로드한다.
    if (SSL_CTX_use_certificate_file(ctx, certpath, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // SSL_CTX 구조체에 개인 키를 로드한다.
    if (SSL_CTX_use_PrivateKey_file(ctx, keypath, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    return 0;
}

int read_ssl(SSL *ssl, char *buffer, int buffer_size)
{
    int bytes = SSL_read(ssl, buffer, buffer_size);
    if (bytes <= 0)
    {
        int error = SSL_get_error(ssl, bytes);
        
        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
        {
            // non-blocking mode
            return 0;
        }
        else
        {
            perror("SSL_read() failed");
            return -1;
        }
    }

    buffer[bytes] = '\0';
    return bytes;
}

int write_ssl(SSL *ssl, const char *buffer, int buffer_size)
{
    int bytes = SSL_write(ssl, buffer, buffer_size);
    if (bytes <= 0)
    {
        int error = SSL_get_error(ssl, bytes);
        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
        {
            // non-blocking mode
            return 0;
        }
        else
        {
            perror("SSL_write() failed");
            return -1;
        }
    }
    return bytes;
}

#endif