#include "storage_nvs.h"

bool StorageNVS::begin(const char* ns, bool readOnly)
{
    return prefs.begin(ns, readOnly);
}

bool StorageNVS::writeString(const char* key, const String& value)
{
    return prefs.putString(key, value) > 0;
}

bool StorageNVS::readString(const char* key, String& value, const String& def)
{
    value = prefs.getString(key, def);
    return true;
}

bool StorageNVS::writeUInt(const char* key, uint32_t value)
{
    return prefs.putUInt(key, value) > 0;
}

bool StorageNVS::readUInt(const char* key, uint32_t& value, uint32_t def)
{
    value = prefs.getUInt(key, def);
    return true;
}

bool StorageNVS::writeUShort(const char* key, uint16_t value)
{
    return prefs.putUShort(key, value) > 0;
}

bool StorageNVS::readUShort(const char* key, uint16_t& value, uint16_t def)
{
    value = prefs.getUShort(key, def);
    return true;
}

bool StorageNVS::writeBool(const char* key, bool value)
{
    return prefs.putBool(key, value);
}

bool StorageNVS::readBool(const char* key, bool& value, bool def)
{
    value = prefs.getBool(key, def);
    return true;
}

bool StorageNVS::writeIP(const char *key, const IPAddress &value)
{
    uint8_t ip[4] = {
        value[0],
        value[1],
        value[2],
        value[3]
    };

    return prefs.putBytes(key, ip, sizeof(ip)) == sizeof(ip);
}

bool StorageNVS::readIP(const char *key, IPAddress &value, const IPAddress &def)
{
    uint8_t ip[4];

    if (prefs.getBytesLength(key) == sizeof(ip))
    {
        if (prefs.getBytes(key, ip, sizeof(ip)) == sizeof(ip))
        {
            value = IPAddress(ip[0], ip[1], ip[2], ip[3]);
            return true;
        }
    }

    value = def;
    return false;
}


bool StorageNVS::remove(const char* key){
    return prefs.remove(key);
}

bool StorageNVS::clear(){
    return prefs.clear();
}