#pragma once
#include <Arduino.h>
#include <Preferences.h>

class StorageNVS
{
public:
    bool begin(const char *ns = "config", bool readOnly = false);

    bool writeString(const char *key, const String &value);
    bool readString(const char *key, String &value, const String &def = "");

    bool writeUInt(const char *key, uint32_t value);
    bool readUInt(const char *key, uint32_t &value, uint32_t def = 0);

    bool writeUShort(const char *key, uint16_t value);
    bool readUShort(const char *key, uint16_t &value, uint16_t def = 0);

    bool writeBool(const char *key, bool value);
    bool readBool(const char *key, bool &value, bool def = false);

    bool writeIP(const char *key, const IPAddress &value);
    bool readIP(const char *key, IPAddress &value, const IPAddress &def = IPAddress(0, 0, 0, 0));

    bool remove(const char *key);
    bool clear();

private:
    Preferences prefs;
};