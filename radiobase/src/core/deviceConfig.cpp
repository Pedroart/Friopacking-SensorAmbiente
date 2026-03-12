#include "deviceConfig.h"
#include "driver/storage_nvs.h"

DeviceConfig Config;

bool DeviceConfig::begin(StorageNVS *_storage)
{
    storage = _storage;
    return (storage != nullptr);
}

SystemConfig DeviceConfig::loadSystem()
{
    SystemConfig cfg;

    if (storage == nullptr)
    {
        return cfg;
    }

    storage->readUShort("sys.ver", cfg.version, DEVICE_VERSION);
    storage->readString("sys.name", cfg.name, DEVICE_NAME);
    storage->readUShort("sys.amb", cfg.ambiente, DEVICE_ID_AMBIENTE);
    storage->readBool("sys.first", cfg.firstLaunch, true);

    return cfg;
}

SystemConfig DeviceConfig::saveSystem(const SystemConfig &in)
{
    if (storage == nullptr)
    {
        return in;
    }

    storage->writeUShort("sys.ver", in.version);
    storage->writeString("sys.name", in.name);
    storage->writeUShort("sys.amb", in.ambiente);
    storage->writeBool("sys.first", in.firstLaunch);

    return in;
}

FeatureConfig DeviceConfig::loadFeatures()
{
    FeatureConfig cfg;

    if (storage == nullptr)
    {
        return cfg;
    }

    storage->readBool("feat.eth", cfg.ethernetEnable, true);
    storage->readBool("feat.ap", cfg.wifiApEnable, true);
    storage->readBool("feat.sta", cfg.wifiStaEnable, false);

    return cfg;
}

FeatureConfig DeviceConfig::saveFeatures(const FeatureConfig &in)
{
    if (storage == nullptr)
    {
        return in;
    }

    storage->writeBool("feat.eth", in.ethernetEnable);
    storage->writeBool("feat.ap", in.wifiApEnable);
    storage->writeBool("feat.sta", in.wifiStaEnable);

    return in;
}

UserConfig DeviceConfig::loadUsers()
{
    UserConfig cfg;

    if (storage == nullptr)
    {
        return cfg;
    }

    cfg.admin.role = UserRole::ADMIN;
    storage->readBool("usr.admin.en", cfg.admin.enabled, true);
    storage->readString("usr.admin.user", cfg.admin.username, "admin");
    storage->readString("usr.admin.pass", cfg.admin.password_hash, USER_PASS_ROOT_SHA256);

    cfg.operador.role = UserRole::OPERATOR;
    storage->readBool("usr.operador.en", cfg.operador.enabled, true);
    storage->readString("usr.operador.user", cfg.operador.username, "operator");
    storage->readString("usr.operador.pass", cfg.operador.password_hash, USER_PASS_DEFAULT_SHA256);

    cfg.viewer.role = UserRole::VIEWER;
    storage->readBool("usr.viewer.en", cfg.viewer.enabled, true);
    storage->readString("usr.viewer.user", cfg.viewer.username, "viewer");
    storage->readString("usr.viewer.pass", cfg.viewer.password_hash, USER_PASS_DEFAULT_SHA256);

    return cfg;
}

UserEntry* DeviceConfig::loadRole(UserConfig& cfg, const String& role){

    if (role == "admin") {
        return &cfg.admin;
    }

    if (role == "operator") {
        return &cfg.operador;
    }

    if (role == "viewer") {
        return &cfg.viewer;
    }

    return nullptr;
}

UserConfig DeviceConfig::saveUsers(const UserConfig &in)
{
    if (storage == nullptr)
    {
        return in;
    }

    storage->writeBool("usr.admin.en", in.admin.enabled);
    storage->writeString("usr.admin.user", in.admin.username);
    storage->writeString("usr.admin.pass", in.admin.password_hash);

    storage->writeBool("usr.operador.en", in.operador.enabled);
    storage->writeString("usr.operador.user", in.operador.username);
    storage->writeString("usr.operador.pass", in.operador.password_hash);

    storage->writeBool("usr.viewer.en", in.viewer.enabled);
    storage->writeString("usr.viewer.user", in.viewer.username);
    storage->writeString("usr.viewer.pass", in.viewer.password_hash);

    return in;
}

NetworkConfig DeviceConfig::loadNetwork()
{
    NetworkConfig cfg;

    if (storage == nullptr)
    {
        return cfg;
    }

    // Ethernet
    storage->readBool("net.eth.dhcp", cfg.eth.net.dhcp, false);
    storage->readIP("net.eth.ip",     cfg.eth.net.ip,      IPAddress(192, 168, ETH_SUBRED, ETH_ID));
    storage->readIP("net.eth.gat",    cfg.eth.net.gateway, IPAddress(192, 168, ETH_SUBRED, 1));
    storage->readIP("net.eth.sub",    cfg.eth.net.subnet,  IPAddress(255, 255, 255, 0));
    storage->readIP("net.eth.dns1",   cfg.eth.net.dns1,    IPAddress(8, 8, 8, 8));
    storage->readIP("net.eth.dns2",   cfg.eth.net.dns2,    IPAddress(1, 1, 1, 1));

    // WiFi AP
    String tmp;

    storage->readString("net.ap.ssid", tmp, AP_NAME);
    snprintf(cfg.ap.ssid, sizeof(cfg.ap.ssid), "%s", tmp.c_str());

    storage->readString("net.ap.pass", tmp, AP_PASS);
    snprintf(cfg.ap.password, sizeof(cfg.ap.password), "%s", tmp.c_str());

    uint16_t ch = 1;
    storage->readUShort("net.ap.chan", ch, 1);
    cfg.ap.channel = ch;

    storage->readBool("net.ap.hid", cfg.ap.hidden, false);

    uint16_t mc = 4;
    storage->readUShort("net.ap.maxc", mc, 4);
    cfg.ap.max_clients = mc;

    storage->readBool("net.ap.dhcp", cfg.ap.net.dhcp, false);
    storage->readIP("net.ap.ip",     cfg.ap.net.ip,      IPAddress(192, 168, 4, 1));
    storage->readIP("net.ap.gat",    cfg.ap.net.gateway, IPAddress(192, 168, 4, 1));
    storage->readIP("net.ap.sub",    cfg.ap.net.subnet,  IPAddress(255, 255, 255, 0));
    storage->readIP("net.ap.dns1",   cfg.ap.net.dns1,    IPAddress(8, 8, 8, 8));
    storage->readIP("net.ap.dns2",   cfg.ap.net.dns2,    IPAddress(1, 1, 1, 1));

    return cfg;
}

NetworkConfig DeviceConfig::saveNetwork(const NetworkConfig &in)
{
    if (storage == nullptr)
    {
        return in;
    }

    // Ethernet
    storage->writeBool("net.eth.dhcp", in.eth.net.dhcp);
    storage->writeIP("net.eth.ip",     in.eth.net.ip);
    storage->writeIP("net.eth.gat",    in.eth.net.gateway);
    storage->writeIP("net.eth.sub",    in.eth.net.subnet);
    storage->writeIP("net.eth.dns1",   in.eth.net.dns1);
    storage->writeIP("net.eth.dns2",   in.eth.net.dns2);

    // WiFi AP
    storage->writeString("net.ap.ssid", String(in.ap.ssid));
    storage->writeString("net.ap.pass", String(in.ap.password));
    storage->writeUShort("net.ap.chan", in.ap.channel);
    storage->writeBool("net.ap.hid",    in.ap.hidden);
    storage->writeUShort("net.ap.maxc", in.ap.max_clients);

    storage->writeBool("net.ap.dhcp", in.ap.net.dhcp);
    storage->writeIP("net.ap.ip",     in.ap.net.ip);
    storage->writeIP("net.ap.gat",    in.ap.net.gateway);
    storage->writeIP("net.ap.sub",    in.ap.net.subnet);
    storage->writeIP("net.ap.dns1",   in.ap.net.dns1);
    storage->writeIP("net.ap.dns2",   in.ap.net.dns2);

    return in;
}