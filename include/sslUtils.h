#ifndef _SSLUTILS_H
#define _SSLUTILS_H

#include "sockC.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_OPENSSL

#define KEY_SIZE        32
#define IV_SIZE         16
#define AES_BLOCK_SIZE  16

int init_ssl();
void cleanup_ssl(SSL **ssl, SSL_CTX **ctx);

void check_ssl_err(int err);

SSL_CTX *create_ctx(int svr);
int configure_ctx(SSL_CTX *ctx, const char *certpath, const char *keypath);
int read_ssl(SSL *ssl, char *buffer, int buffer_size);
int write_ssl(SSL *ssl, const char *buffer, int buffer_size);

void SHA256_encrypt(const unsigned char *input, int len, unsigned char *out);

void AES256CBC_encrypt(const unsigned char *plain, int plainlen, unsigned char *key, unsigned char *iv, unsigned char *cipher, int *cipherlen);
void AES256CBC_decrypt(const unsigned char *cipher, int cipherlen, unsigned char *key, unsigned char *iv, unsigned char *plain, int *plainlen);

#endif

#ifdef __cplusplus
}
#endif

#endif