#include "crypto_lib.h"
#include "mbedtls/aes.h"

mbedtls_aes_context aes_ctx;

bool aes_init_key(const uint8_t key[16]){
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
    return true;
}

bool decrypt_block(const uint8_t in[16], uint8_t out[16])
{
    return mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, in, out) == 0;
}

void aes_cleanup()
{
    mbedtls_aes_free(&aes_ctx);
}