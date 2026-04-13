#include "http_routes.h"
#include <WiFi.h>
#include <AsyncWebServer_ESP32_SC_W5500.h>
#include "responseJson.h"
#include "core/appState.h"

static void fillNetworkStatus(JsonDocument &doc)
{
    JsonObject featuresObj = doc["features"].to<JsonObject>();
    featuresObj["ethernet_enable"] = feature.ethernetEnable;
    featuresObj["wifi_ap_enable"] = feature.wifiApEnable;
    featuresObj["wifi_sta_enable"] = feature.wifiStaEnable;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["mode"] = static_cast<int>(WiFi.getMode());
    wifi["sta_status"] = static_cast<int>(WiFi.status());
    wifi["sta_connected"] = WiFi.status() == WL_CONNECTED;
    wifi["sta_ip"] = WiFi.localIP().toString();
    wifi["ap_ip"] = WiFi.softAPIP().toString();
    wifi["ap_clients"] = WiFi.softAPgetStationNum();

    JsonObject eth = doc["ethernet"].to<JsonObject>();
    eth["enabled"] = feature.ethernetEnable;
    eth["ip"] = ETH.localIP().toString();
    eth["link_up"] = ETH.linkUp();
}

static void fillNetworkConfig(JsonDocument &doc, bool includeSecrets = true)
{
    JsonObject eth = doc["eth"].to<JsonObject>();
    ipToJson(eth["net"].to<JsonObject>(), network.eth.net);

    JsonObject ap = doc["ap"].to<JsonObject>();
    ap["ssid"] = network.ap.ssid;
    ap["channel"] = network.ap.channel;
    ap["hidden"] = network.ap.hidden;
    ap["max_clients"] = network.ap.max_clients;
    ap["password"] = includeSecrets ? String(network.ap.password) : String("***");
    ipToJson(ap["net"].to<JsonObject>(), network.ap.net);

    JsonObject sta = doc["sta"].to<JsonObject>();
    sta["ssid"] = network.sta.ssid;
    sta["password"] = includeSecrets ? String(network.sta.password) : String("***");
    ipToJson(sta["net"].to<JsonObject>(), network.sta.net);
}

static void sendApplyResult(
    AsyncWebServerRequest *request,
    bool applied,
    const char *message)
{
    JsonDocument doc;
    JsonObject data = createResponse(doc, applied, message);
    data["applied"] = applied;
    data["requires_reboot"] = false;

    JsonDocument statusDoc;
    fillNetworkStatus(statusDoc);
    data["status"] = statusDoc.as<JsonVariantConst>();

    sendJson(request, applied ? 200 : 500, doc);
}

static void markNetworkApplyFailure(const char *error)
{
    bootStatus.networkApplied = false;
    bootStatus.lastError = error;
    refreshBootReady();
}

static void markNetworkApplySuccess()
{
    bootStatus.networkApplied = true;
    refreshBootReady();
    if (bootStatus.ready)
    {
        bootStatus.lastError = "";
    }
}

static void sendFeatureConfig(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    JsonObject data = createResponse(doc, true);
    JsonObject features = data["features"].to<JsonObject>();
    features["ethernet_enable"] = feature.ethernetEnable;
    features["wifi_ap_enable"] = feature.wifiApEnable;
    features["wifi_sta_enable"] = feature.wifiStaEnable;
    sendJson(request, 200, doc);
}

static void handleFeatureUpdate(AsyncWebServerRequest *request, uint8_t *payload, size_t len)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);

    if (err)
    {
        sendError(request, 400, "invalid_json");
        return;
    }

    FeatureConfig cfg = feature;

    if (!doc["ethernet_enable"].isNull())
        cfg.ethernetEnable = doc["ethernet_enable"].as<bool>();

    if (!doc["wifi_ap_enable"].isNull())
        cfg.wifiApEnable = doc["wifi_ap_enable"].as<bool>();

    if (!doc["wifi_sta_enable"].isNull())
        cfg.wifiStaEnable = doc["wifi_sta_enable"].as<bool>();

    if (!applyNetworkConfig(network, cfg))
    {
        markNetworkApplyFailure("feature_apply_failed");
        sendApplyResult(request, false, "No se pudo aplicar la configuracion de features");
        return;
    }

    feature = cfg;
    markNetworkApplySuccess();
    Config.saveFeatures(feature);

    sendApplyResult(request, true, "Configuracion de features actualizada");
}

static void handleEthernetUpdate(AsyncWebServerRequest *request, uint8_t *payload, size_t len)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);

    if (err)
    {
        sendError(request, 400, "invalid_json");
        return;
    }

    JsonObject net = doc["net"].as<JsonObject>();
    if (net.isNull())
    {
        sendError(request, 400, "missing_net");
        return;
    }

    EthernetConfig cfg = network.eth;
    String error;

    if (!jsonToIpSettings(net, cfg.net, error))
    {
        sendError(request, 400, error);
        return;
    }

    NetworkConfig nextNetwork = network;
    nextNetwork.eth = cfg;

    if (!applyNetworkConfig(nextNetwork, feature))
    {
        markNetworkApplyFailure("ethernet_apply_failed");
        sendApplyResult(request, false, "No se pudo aplicar la configuracion Ethernet");
        return;
    }

    network = nextNetwork;
    markNetworkApplySuccess();
    Config.saveNetwork(network);

    sendApplyResult(request, true, "Configuracion Ethernet actualizada");
}

static void handleApUpdate(AsyncWebServerRequest *request, uint8_t *payload, size_t len)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);

    if (err)
    {
        sendError(request, 400, "invalid_json");
        return;
    }

    WifiApConfig cfg = network.ap;

    String ssid = doc["ssid"] | "";
    String password = doc["password"] | "";
    int channel = doc["channel"] | cfg.channel;
    bool hidden = doc["hidden"] | cfg.hidden;
    int maxClients = doc["max_clients"] | cfg.max_clients;

    if (!ssid.isEmpty())
    {
        strncpy(cfg.ssid, ssid.c_str(), sizeof(cfg.ssid) - 1);
        cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
    }

    if (!password.isEmpty())
    {
        strncpy(cfg.password, password.c_str(), sizeof(cfg.password) - 1);
        cfg.password[sizeof(cfg.password) - 1] = '\0';
    }

    if (channel < 1 || channel > 13)
    {
        sendError(request, 400, "channel_invalido");
        return;
    }

    if (maxClients < 1 || maxClients > 8)
    {
        sendError(request, 400, "max_clients_invalido");
        return;
    }

    cfg.channel = static_cast<uint8_t>(channel);
    cfg.hidden = hidden;
    cfg.max_clients = static_cast<uint8_t>(maxClients);

    JsonObject net = doc["net"].as<JsonObject>();
    if (!net.isNull())
    {
        String error;
        if (!jsonToIpSettings(net, cfg.net, error))
        {
            sendError(request, 400, error);
            return;
        }
    }

    NetworkConfig nextNetwork = network;
    nextNetwork.ap = cfg;

    if (!applyNetworkConfig(nextNetwork, feature))
    {
        markNetworkApplyFailure("ap_apply_failed");
        sendApplyResult(request, false, "No se pudo aplicar la configuracion AP");
        return;
    }

    network = nextNetwork;
    markNetworkApplySuccess();
    Config.saveNetwork(network);

    sendApplyResult(request, true, "Configuracion AP actualizada");
}

static void handleStaUpdate(AsyncWebServerRequest *request, uint8_t *payload, size_t len)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);

    if (err)
    {
        sendError(request, 400, "invalid_json");
        return;
    }

    WifiStaConfig cfg = network.sta;

    String ssid = doc["ssid"] | "";
    String password = doc["password"] | "";

    if (!ssid.isEmpty())
    {
        strncpy(cfg.ssid, ssid.c_str(), sizeof(cfg.ssid) - 1);
        cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
    }

    if (!password.isEmpty())
    {
        strncpy(cfg.password, password.c_str(), sizeof(cfg.password) - 1);
        cfg.password[sizeof(cfg.password) - 1] = '\0';
    }

    JsonObject net = doc["net"].as<JsonObject>();
    if (!net.isNull())
    {
        String error;
        if (!jsonToIpSettings(net, cfg.net, error))
        {
            sendError(request, 400, error);
            return;
        }
    }

    NetworkConfig nextNetwork = network;
    nextNetwork.sta = cfg;

    if (!applyNetworkConfig(nextNetwork, feature))
    {
        markNetworkApplyFailure("sta_apply_failed");
        sendApplyResult(request, false, "No se pudo aplicar la configuracion STA");
        return;
    }

    network = nextNetwork;
    markNetworkApplySuccess();
    Config.saveNetwork(network);

    sendApplyResult(request, true, "Configuracion STA actualizada");
}

void registerHttpRoutes(AsyncWebServer &server)
{
    server.on("/api/system/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;
        JsonObject data = createResponse(doc, true);

        data["ready"] = bootStatus.ready;
        data["health"] = bootStatus.ready ? "ok" : "degraded";
        data["uptime_ms"] = millis();
        data["heap_free"] = ESP.getFreeHeap();
        data["heap_min_free"] = ESP.getMinFreeHeap();

        JsonObject device = data["device"].to<JsonObject>();
        device["name"] = sys.name;
        device["version"] = sys.version;
        device["ambiente"] = sys.ambiente;
        device["first_launch"] = sys.firstLaunch;

        JsonObject boot = data["boot"].to<JsonObject>();
        boot["storage_ready"] = bootStatus.storageReady;
        boot["filesystem_ready"] = bootStatus.filesystemReady;
        boot["config_loaded"] = bootStatus.configLoaded;
        boot["beacon_registry_ready"] = bootStatus.beaconRegistryReady;
        boot["slot_manager_ready"] = bootStatus.slotManagerReady;
        boot["network_applied"] = bootStatus.networkApplied;
        boot["web_ready"] = bootStatus.webReady;
        boot["ble_ready"] = bootStatus.bleReady;
        boot["boot_completed_ms"] = bootStatus.bootCompletedMs;
        boot["last_error"] = bootStatus.lastError;

        JsonObject runtime = data["runtime"].to<JsonObject>();
        runtime["data_queue_ready"] = dataQ != nullptr;
        runtime["adv_task_ready"] = advTaskHandle != nullptr;
        runtime["beacon_logic_task_ready"] = beaconLogicTaskHandle != nullptr;

        JsonDocument networkDoc;
        fillNetworkStatus(networkDoc);
        data["network"] = networkDoc.as<JsonVariantConst>();

        sendJson(request, 200, doc); });

    server.on("/api/network/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;
        JsonObject data = createResponse(doc, true);

        JsonDocument statusDoc;
        fillNetworkStatus(statusDoc);
        data.set(statusDoc.as<JsonVariantConst>());
        JsonObject device = data["device"].to<JsonObject>();
        device["name"] = sys.name;
        device["version"] = sys.version;
        device["ambiente"] = sys.ambiente;
        device["uptime_ms"] = millis();
        device["heap_free"] = ESP.getFreeHeap();
        device["heap_min_free"] = ESP.getMinFreeHeap();

        sendJson(request, 200, doc); });

    server.on("/api/device/info", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;
        JsonObject data = createResponse(doc, true);
        data["device"] = "gateway";
        sendJson(request, 200, doc); });

    server.on("/api/ble/stats", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        BlePipelineStats stats = advertising.stats();

        JsonDocument doc;
        JsonObject data = createResponse(doc, true);
        JsonObject ble = data["ble"].to<JsonObject>();

        ble["adv_received"] = stats.adv_received;
        ble["adv_dropped"] = stats.adv_dropped;
        ble["adv_decrypt_fail"] = stats.adv_decrypt_fail;
        ble["data_enqueued"] = stats.data_enqueued;
        ble["data_dropped"] = stats.data_dropped;
        ble["data_processed"] = stats.data_processed;
        ble["mapped_updates"] = stats.mapped_updates;
        ble["direct_updates"] = stats.direct_updates;
        ble["registry_updates"] = stats.registry_updates;
        ble["registry_new"] = stats.registry_new;
        ble["current_adv_depth"] = stats.current_adv_depth;
        ble["current_data_depth"] = stats.current_data_depth;
        ble["max_adv_depth"] = stats.max_adv_depth;
        ble["max_data_depth"] = stats.max_data_depth;
        ble["max_end_to_end_ms"] = stats.max_end_to_end_ms;

        sendJson(request, 200, doc); });

    server.on("/api/ble/stats/reset", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        advertising.resetStats();
        sendSuccess(request, "BLE stats reiniciadas"); });
}

void registerFeatureRoutes(AsyncWebServer &server)
{
    server.on("/api/feature", HTTP_GET, sendFeatureConfig);
    server.on("/api/features", HTTP_GET, sendFeatureConfig);

    server.on("/api/features", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleFeatureUpdate(request, data, len); });

    server.on("/api/features", HTTP_PATCH, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleFeatureUpdate(request, data, len); });
}

void registerNetworkRoutes(AsyncWebServer &server)
{
    auto sendNetworkConfig = [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;
        JsonObject data = createResponse(doc, true);
        JsonDocument configDoc;
        fillNetworkConfig(configDoc);
        data.set(configDoc.as<JsonVariantConst>());
        sendJson(request, 200, doc); 
    };
    

    server.on("/api/network", HTTP_GET, sendNetworkConfig);
    server.on("/api/network/config", HTTP_GET, sendNetworkConfig);

    server.on("/api/network/ethernet", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleEthernetUpdate(request, data, len); });
    server.on("/api/network/ethernet", HTTP_PATCH, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleEthernetUpdate(request, data, len); });

    server.on("/api/network/ap", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleApUpdate(request, data, len); });
    server.on("/api/network/ap", HTTP_PATCH, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleApUpdate(request, data, len); });

    server.on("/api/network/sta", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleStaUpdate(request, data, len); });
    server.on("/api/network/sta", HTTP_PATCH, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              { handleStaUpdate(request, data, len); });
}

void registerBeaconRoutes(AsyncWebServer &server)
{
    server.on("/api/map", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    JsonDocument doc;
    JsonObject data = createResponse(doc, true);
    JsonArray arr = data["map"].to<JsonArray>();

    if (slotManager.lockMap())
    {
        BeaconMapEntry *map = slotManager.getMap();

        for (int i = 0; i < MAX_SLOTS; ++i)
        {
            JsonObject obj = arr.add<JsonObject>();
            obj["index"] = i;
            obj["enabled"] = map[i].enabled;
            obj["addr"] = uint64ToHex(map[i].addr);
            obj["slot"] = map[i].slot;
        }

        slotManager.unlockMap();
    }

    sendJson(request, 200, doc); });

    server.on("/api/map", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
              {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);

    if (err)
    {
        sendError(request, 400, "invalid_json");
        return;
    }

    int index = doc["index"] | -1;
    bool enabled = doc["enabled"] | false;
    String addrStr = doc["addr"] | "";
    int slot = doc["slot"] | -1;

    if (index < 0 || index >= MAX_SLOTS || slot < 0 || slot >= MAX_SLOTS)
    {
        sendError(request, 400, "invalid_index_or_slot");
        return;
    }

    uint64_t addr = hexToUint64(addrStr);

    if (!slotManager.setMapEntry(index, addr, slot, enabled))
    {
        sendError(request, 500, "save_failed");
        return;
    }

    sendSuccess(request, "Mapa actualizado"); });

    server.on("/api/map", HTTP_DELETE, [](AsyncWebServerRequest *request)
              {
    if (!slotManager.clearMap())
    {
        sendError(request, 500, "clear_failed");
        return;
    }

    sendSuccess(request, "Mapa limpiado"); });

    server.on("/api/slots", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    JsonDocument doc;
    JsonObject data = createResponse(doc, true);
    JsonArray arr = data["slots"].to<JsonArray>();

    if (slotManager.lockSlots())
    {
        SlotState *slots = slotManager.getSlots();

        for (int i = 0; i < MAX_SLOTS; ++i)
        {
            JsonObject obj = arr.add<JsonObject>();
            obj["index"] = i;
            obj["used"] = slots[i].used;
            obj["addr"] = obj["addr"] = addrToHex(slots[i].addr);;
            obj["last_seen_ms"] = slots[i].last_seen_ms;

            JsonObject last = obj["last"].to<JsonObject>();
            last["environment_id"] = slots[i].last.environment_id;
            last["device_id"] = slots[i].last.device_id;
        }

        slotManager.unlockSlots();
    }

    sendJson(request, 200, doc); });

    server.on("/api/beacons", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;
        JsonObject data = createResponse(doc, true);
        JsonArray arr = data["items"].to<JsonArray>();
        DiscoveredBeacon snapshot[MAX_DISCOVERED_BEACONS]{};

        if (!beaconRegistry.snapshot(snapshot, MAX_DISCOVERED_BEACONS))
        {
            sendError(request, 500, "beacon_snapshot_failed");
            return;
        }

        for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
        {
            if (!snapshot[i].used) continue;

            JsonObject obj = arr.add<JsonObject>();
            obj["index"] = i;
            obj["used"] = snapshot[i].used;
            obj["is_new"] = snapshot[i].isNew;
            obj["addr"] = addrToHex(snapshot[i].addr);
            obj["environment_id"] = snapshot[i].environment_id;
            obj["device_id"] = snapshot[i].device_id;
            obj["rssi"] = snapshot[i].rssi;
            obj["first_seen_ms"] = snapshot[i].first_seen_ms;
            obj["last_seen_ms"] = snapshot[i].last_seen_ms;
            obj["seen_count"] = snapshot[i].seen_count;
        }

        data["max"] = MAX_DISCOVERED_BEACONS;
        data["new_count"] = beaconRegistry.countNew();

        sendJson(request, 200, doc); });
}  
