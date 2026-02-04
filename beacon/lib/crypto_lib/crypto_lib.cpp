#include "crypto_lib.h"
#include <string.h>
#include "nrf_soc.h"

namespace crypto_lib {

size_t padding_pkcs7(uint8_t* buffer, size_t data_len, size_t block_size)
{
  if (!buffer || block_size == 0) return data_len;

  size_t pad = block_size - (data_len % block_size);
  if (pad == 0) pad = block_size;

  for (size_t i = 0; i < pad; i++) {
    buffer[data_len + i] = (uint8_t)pad;
  }
  return data_len + pad;
}

size_t unpadding_pkcs7(uint8_t* buffer, size_t len, size_t block_size)
{
  if (!buffer || len == 0 || block_size == 0) return len;
  if ((len % block_size) != 0) return len;

  uint8_t pad = buffer[len - 1];
  if (pad == 0 || pad > block_size) return len;

  // valida bytes de padding
  for (size_t i = 0; i < pad; i++) {
    if (buffer[len - 1 - i] != pad) return len;
  }
  return len - pad;
}

// -------------------- XOR --------------------
void xor_buf(uint8_t* out, const uint8_t* a, const uint8_t* b, size_t len)
{
  if (!out || !a || !b) return;
  for (size_t i = 0; i < len; i++) out[i] = a[i] ^ b[i];
}

void xor_inplace(uint8_t* data, const uint8_t* stream, size_t len)
{
  if (!data || !stream) return;
  for (size_t i = 0; i < len; i++) data[i] ^= stream[i];
}

// -------------------- AES-ECB (SoftDevice) --------------------
bool aes_ecb_encrypt_sd(const uint8_t key[16], const uint8_t in[16], uint8_t out[16])
{
  if (!key || !in || !out) return false;

  nrf_ecb_hal_data_t ecb;
  memcpy(ecb.key, key, 16);
  memcpy(ecb.cleartext, in, 16);

  uint32_t err = sd_ecb_block_encrypt(&ecb);
  if (err != NRF_SUCCESS) return false;

  memcpy(out, ecb.ciphertext, 16);
  return true;
}

// -------------------- Encrypt: ECB + PKCS#7 --------------------
bool encrypt_ecb_pkcs7(const uint8_t key[16],
                       const uint8_t* in, size_t in_len,
                       uint8_t* out, size_t out_cap,
                       size_t* out_len_out,
                       size_t block_size)
{
  if (!key || !in || !out || !out_len_out) return false;
  if (block_size != 16) {
    // AES-ECB del SoftDevice trabaja en 16 bytes. Si quieres otro bloque, habría que cambiar motor.
    return false;
  }

  // tamaño final con PKCS#7 siempre suma al menos 1 bloque
  size_t padded_len = ((in_len / 16) + 1) * 16;
  if (out_cap < padded_len) return false;

  // Copiar input y aplicar padding sobre el buffer de salida (en RAM)
  memcpy(out, in, in_len);
  size_t final_len = padding_pkcs7(out, in_len, 16); // debe dar padded_len
  if (final_len != padded_len) {
    // por seguridad, aunque matemáticamente debería coincidir
    if (final_len > out_cap) return false;
    padded_len = final_len;
  }

  // Cifrar bloque por bloque (ECB)
  for (size_t off = 0; off < padded_len; off += 16)
  {
    uint8_t enc[16];
    if (!aes_ecb_encrypt_sd(key, out + off, enc)) return false;
    memcpy(out + off, enc, 16);
  }

  *out_len_out = padded_len;
  return true;
}

} // namespace crypto_lib
