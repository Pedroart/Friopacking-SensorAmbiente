#include "services/web_service.h"
#include "core/appState.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.print("Arranque de Sistema");

    bootStatus = {};

    bootStatus.storageReady = storage.begin();
    if (!bootStatus.storageReady)
    {
        bootStatus.lastError = "storage_begin_failed";
    }

    Config.begin(&storage);

    bootStatus.filesystemReady = LittleFS.begin(true);
    if (!bootStatus.filesystemReady && bootStatus.lastError.isEmpty())
    {
        bootStatus.lastError = "littlefs_begin_failed";
    }

    sys = Config.loadSystem();
    feature = Config.loadFeatures();
    users = Config.loadUsers();
    network = Config.loadNetwork();
    bootStatus.configLoaded = true;

    beaconRegistry.begin();
    bootStatus.beaconRegistryReady = true;

    bootStatus.slotManagerReady = slotManager.begin();
    if (!bootStatus.slotManagerReady && bootStatus.lastError.isEmpty())
    {
        bootStatus.lastError = "slot_manager_begin_failed";
    }

    bootStatus.networkApplied = applyNetworkConfig(network, feature);
    if (!bootStatus.networkApplied && bootStatus.lastError.isEmpty())
    {
        bootStatus.lastError = "network_apply_failed";
    }
    
    bootStatus.webReady = webService.begin();
    if (!bootStatus.webReady && bootStatus.lastError.isEmpty())
    {
        bootStatus.lastError = "web_service_begin_failed";
    }

    bootStatus.bleReady = advertising.begin();
    if (!bootStatus.bleReady && bootStatus.lastError.isEmpty())
    {
        bootStatus.lastError = "ble_begin_failed";
    }

    refreshBootReady();
    bootStatus.bootCompletedMs = millis();
}

void loop() {
    webService.loop();
}
