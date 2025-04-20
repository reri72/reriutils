#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reriutils.h"

#if HAVE_OPENSSL
void sha256_test()
{
    unsigned char input[16] = "hi world";
    unsigned char hash[EVP_MAX_MD_SIZE] = {0,};
    char *base64_hash = NULL;

    int len = strlen(input);
    
    printf("============== %s print! ==============\n", __FUNCTION__);
    {
        SHA256_encrypt(input, len, hash);

        printf("Input: %s \n", input);
    
        printf("SHA-256 Hash: ");
        int i = 0;
        for (i = 0; i < EVP_MD_size(EVP_sha256()); i++)
        {
            printf("%02x", hash[i]);
        }
        printf("\n");

        base64_hash = BASE64_encode(hash, (EVP_MAX_MD_SIZE/2));
        if (base64_hash != NULL)
        {
            printf("BASE64 encoding : %s \n", base64_hash);
            free(base64_hash);
        }
        
    }
    printf("===========================================\n\n");
}

void aes256cbc_test()
{
    unsigned char plain[32] = "bye world!";
    int plainlen = strlen(plain);

    unsigned char key[KEY_SIZE] = {0,};
    unsigned char iv[IV_SIZE] = {0,};

    RAND_bytes(key, KEY_SIZE);
    RAND_bytes(iv, IV_SIZE);

    int cipherlen = 0;
    unsigned char cipher[plainlen + AES_BLOCK_SIZE];
    AES256CBC_encrypt(plain, plainlen, key, iv, cipher, &cipherlen);

    int decrypted_len = 0;
    unsigned char decrypted_plaintext[cipherlen];
    AES256CBC_decrypt(cipher, cipherlen, key, iv, decrypted_plaintext, &decrypted_len);

    printf("============== %s print! ==============\n", __FUNCTION__);
    {
        printf("plain text : %s\n", plain);

        printf("cipher text : ");
        for (int i = 0; i < cipherlen; i++)
        {
            printf("%02x", cipher[i]);
        }
        printf("\n");

        printf("decrypted : ");
        for(int i = 0; i < decrypted_len; i++)
        {
            printf("%c", decrypted_plaintext[i]);
        }
        printf("\n");
    }
    printf("=================================================\n\n");
}
#endif

int main(int argc, char **argv)
{
#if HAVE_OPENSSL

    sha256_test();
    aes256cbc_test();
    
#endif
    return 0;
}