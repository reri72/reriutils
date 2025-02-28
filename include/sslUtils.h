#ifndef _SSLUTILS_H
#define _SSLUTILS_H

#include "sockC.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_OPENSSL
void init_ssl();
void cleanup_ssl(SSL **ssl, SSL_CTX **ctx);

void check_ssl_err(int err);

SSL_CTX *create_ctx(int svr);
int configure_ctx(SSL_CTX *ctx, const char *certpath, const char *keypath);
#endif

#ifdef __cplusplus
}
#endif

#endif