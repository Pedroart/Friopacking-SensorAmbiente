#pragma once

#include <Arduino.h>

bool kv_begin();

bool     kv_set_u32(const char *key, uint32_t value);
uint32_t kv_get_u32(const char *key, uint32_t defaultValue);

bool     kv_set_i32(const char *key, int32_t value);
int32_t  kv_get_i32(const char *key, int32_t defaultValue);

bool kv_set_bool(const char *key, bool value);
bool kv_get_bool(const char *key, bool defaultValue);


