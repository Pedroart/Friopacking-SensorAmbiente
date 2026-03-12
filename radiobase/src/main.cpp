#include "services/web_service.h"
#include "core/appState.h"

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