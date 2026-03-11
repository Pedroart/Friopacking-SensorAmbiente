#include "driver/network_manager.h"
#include "driver/storage_nvs.h"
#include "core/deviceConfig.h"
#include "services/web_service.h"


StorageNVS storage;
SystemConfig sys;
FeatureConfig feature;
UserConfig users;
NetworkConfig network;

WebService webService;

void setup() {

    storage.begin();
    Config.begin(&storage);

    sys = Config.loadSystem();
    feature = Config.loadFeatures();
    users = Config.loadUsers();
    network = Config.loadNetwork();

    applyWifiConfig(network,feature);
    
    webService.begin();
}

void loop() {
    webService.loop();
}