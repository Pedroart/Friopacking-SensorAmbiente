// ===== crypto_lib.h =====
#pragma once
#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

namespace crypto_lib {

size_t padding_pkcs7(uint8_t* buffer, size_t data_len, size_t block_size = 16);

size_t unpadding_pkcs7(uint8_t* buffer, size_t len, size_t block_size = 16);

void xor_buf(uint8_t* out, const uint8_t* a, const uint8_t* b, size_t len);
void xor_inplace(uint8_t* data, const uint8_t* stream, size_t len);

// -------------------- AES-ECB (SoftDevice) --------------------
// Cifra 1 bloque de 16 bytes con AES-128 ECB usando SoftDevice
bool aes_ecb_encrypt_sd(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);

// -------------------- Encriptación ECB + PKCS7 --------------------
// Encripta "in" de tamaño in_len, aplica PKCS7 y cifra en ECB.
// - out debe tener tamaño >= ((in_len/16)+1)*16
// - out_len_out devuelve el tamaño final cifrado (múltiplo de 16)
//
// Retorna false si:
// - parámetros inválidos
// - falla sd_ecb_block_encrypt
bool encrypt_ecb_pkcs7(const uint8_t key[16],
                       const uint8_t* in, size_t in_len,
                       uint8_t* out, size_t out_cap,
                       size_t* out_len_out,
                       size_t block_size = 16);

}
