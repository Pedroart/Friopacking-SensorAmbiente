#pragma once
#include <Arduino.h>

bool aes_init_key(const uint8_t key[16]);
bool decrypt_block(const uint8_t in[16], uint8_t out[16]);
void aes_cleanup();