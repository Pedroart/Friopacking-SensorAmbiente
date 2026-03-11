#pragma once
#include <Arduino.h>
#include "networkConfig.h"
#include "userConfig.h"
#include "driver/storage_nvs.h"
#include "config.h"

struct SystemConfig {
    uint16_t version;
    String name;
    uint16_t ambiente;
    bool firstLaunch;
};

struct FeatureConfig {
    bool ethernetEnable;
    bool wifiApEnable;
    bool wifiStaEnable;
};

class DeviceConfig {
public:

    bool begin(StorageNVS* storage);

    SystemConfig loadSystem();
    SystemConfig saveSystem(const SystemConfig& in);

    FeatureConfig loadFeatures();
    FeatureConfig saveFeatures(const FeatureConfig& in);

    NetworkConfig loadNetwork();
    NetworkConfig saveNetwork(const NetworkConfig& in);

    UserConfig loadUsers();
    UserConfig saveUsers(const UserConfig& in);

private:
    StorageNVS* storage = nullptr;
};

extern DeviceConfig Config;