#include "sslUtils.h"

#if HAVE_OPENSSL

void init_ssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_ssl(SSL **ssl, SSL_CTX **ctx)
{
    if (*ssl != NULL)
        SSL_free(*ssl);

    if (*ctx != NULL)
        SSL_CTX_free(*ctx);
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

#endif