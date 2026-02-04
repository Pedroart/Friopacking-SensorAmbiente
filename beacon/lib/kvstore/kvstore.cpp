#include "kvstore.h"

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

static File file(InternalFS);
static bool fs_ready = false;

bool kv_begin() {
  if (!fs_ready) {
    fs_ready = InternalFS.begin();
  }
  return fs_ready;
}

static bool writeBytes(const char *key, const void *data, size_t len) {
  if (!kv_begin()) return false;

  // CREATE + WRITE + TRUNCATE (robusto)
  if (!file.open(key, FILE_O_WRITE)) return false;

  file.write((const uint8_t*)data, len);
  file.close();
  return true;
}



static bool readBytes(const char *key, void *data, size_t len) {
  if (!kv_begin()) return false;

  if (!file.open(key, FILE_O_READ)) return false;

  if (file.size() != (int)len) {
    file.close();
    return false;
  }

  file.read((uint8_t*)data, len);
  file.close();
  return true;
}

// ======================= U32 =======================

bool kv_set_u32(const char *key, uint32_t value) {
  return writeBytes(key, &value, sizeof(value));
}

uint32_t kv_get_u32(const char *key, uint32_t defaultValue) {
  uint32_t v;
  if (readBytes(key, &v, sizeof(v))) return v;
  return defaultValue;
}

// ======================= I32 =======================

bool kv_set_i32(const char *key, int32_t value) {
  return writeBytes(key, &value, sizeof(value));
}

int32_t kv_get_i32(const char *key, int32_t defaultValue) {
  int32_t v;
  if (readBytes(key, &v, sizeof(v))) return v;
  return defaultValue;
}

// ======================= BOOL ======================

bool kv_set_bool(const char *key, bool value) {
  uint8_t v = value ? 1 : 0;
  return writeBytes(key, &v, sizeof(v));
}

bool kv_get_bool(const char *key, bool defaultValue) {
  uint8_t v;
  if (readBytes(key, &v, sizeof(v))) return v != 0;
  return defaultValue;
}
