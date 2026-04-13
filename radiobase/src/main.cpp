#include "services/web_service.h"
#include "core/appState.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.print("Arranque de Sistema");

    storage.begin();
    Config.begin(&storage);

    LittleFS.begin(true);

    sys = Config.loadSystem();
    feature = Config.loadFeatures();
    users = Config.loadUsers();
    network = Config.loadNetwork();

    beaconRegistry.begin();
    slotManager.begin();

    applyNetworkConfig(network,feature);
    
    webService.begin();

    advertising.begin();
    
}

void loop() {
    webService.loop();
}
