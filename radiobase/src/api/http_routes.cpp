#include "http_routes.h"
#include <WiFi.h>
#include "responseJson.h"
#include "core/appState.h"

void registerHttpRoutes(AsyncWebServer &server)
{
    server.on("/api/network/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = "{";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
        json += "}";

        request->send(200, "application/json", json); });

    server.on("/api/device/info", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", "{\"device\":\"gateway\"}"); });

    server.on("/api/ble/stats", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        BlePipelineStats stats = advertising.stats();

        JsonDocument doc;
        JsonObject ble = doc["ble"].to<JsonObject>();

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
        request->send(200, "application/json", "{\"ok\":true}"); });
}

void registerFeatureRoutes(AsyncWebServer &server)
{
    server.on("/api/feature", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;
        JsonObject features = doc["features"].to<JsonObject>();
        features["ethernet_enable"] = feature.ethernetEnable;
        features["wifi_ap_enable"] = feature.wifiApEnable;
        features["wifi_sta_enable"] = feature.wifiStaEnable;

        sendJson(request, 200, doc); });

    server.on("/api/features", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

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

            feature = cfg;

            Config.saveFeatures(feature);   // o el metodo que uses para persistirlo

            sendSuccess(request, "Configuracion de features actualizada"); });
}

void registerNetworkRoutes(AsyncWebServer &server)
{
    server.on("/api/network", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument doc;

        JsonObject eth = doc["eth"].to<JsonObject>();
        ipToJson(eth["net"].to<JsonObject>(), network.eth.net);

        JsonObject ap = doc["ap"].to<JsonObject>();
        ap["ssid"] = network.ap.ssid;
        ap["password"] = network.ap.password;
        ap["channel"] = network.ap.channel;
        ap["hidden"] = network.ap.hidden;
        ap["max_clients"] = network.ap.max_clients;
        ipToJson(ap["net"].to<JsonObject>(), network.ap.net);

        JsonObject sta = doc["sta"].to<JsonObject>();
        sta["ssid"] = network.sta.ssid;
        sta["password"] = network.sta.password;
        ipToJson(sta["net"].to<JsonObject>(), network.sta.net);

        sendJson(request, 200, doc); });

    server.on("/api/network/ethernet", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

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

            network.eth = cfg;

            Config.saveNetwork(network);

            sendSuccess(request, "Configuracion Ethernet actualizada"); });

    server.on("/api/network/ap", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

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
                sendError(request, 400, "channel invalido");
                return;
            }

            if (maxClients < 1 || maxClients > 8)
            {
                sendError(request, 400, "max_clients invalido");
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

            network.ap = cfg;

            Config.saveNetwork(network);

            sendSuccess(request, "Configuracion AP actualizada"); });

    server.on("/api/network/sta", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

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

            network.sta = cfg;

            Config.saveNetwork(network);

            sendSuccess(request, "Configuracion STA actualizada"); });
}

void registerBeaconRoutes(AsyncWebServer &server)
{
    server.on("/api/map", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    JsonDocument doc;
    JsonArray arr = doc["map"].to<JsonArray>();

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
        request->send(400, "application/json", "{\"error\":\"invalid json\"}");
        return;
    }

    int index = doc["index"] | -1;
    bool enabled = doc["enabled"] | false;
    String addrStr = doc["addr"] | "";
    int slot = doc["slot"] | -1;

    if (index < 0 || index >= MAX_SLOTS || slot < 0 || slot >= MAX_SLOTS)
    {
        request->send(400, "application/json", "{\"error\":\"invalid index/slot\"}");
        return;
    }

    uint64_t addr = hexToUint64(addrStr);

    if (!slotManager.setMapEntry(index, addr, slot, enabled))
    {
        request->send(500, "application/json", "{\"error\":\"save failed\"}");
        return;
    }

    request->send(200, "application/json", "{\"ok\":true}"); });

    server.on("/api/map", HTTP_DELETE, [](AsyncWebServerRequest *request)
              {
    if (!slotManager.clearMap())
    {
        request->send(500, "application/json", "{\"error\":\"clear failed\"}");
        return;
    }

    request->send(200, "application/json", "{\"ok\":true}"); });

    server.on("/api/slots", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    JsonDocument doc;
    JsonArray arr = doc["slots"].to<JsonArray>();

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
        JsonArray arr = doc["items"].to<JsonArray>();

        const DiscoveredBeacon *items = beaconRegistry.items();

        for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
        {
            if (!items[i].used) continue;

            JsonObject obj = arr.add<JsonObject>();
            obj["index"] = i;
            obj["used"] = items[i].used;
            obj["is_new"] = items[i].isNew;
            obj["addr"] = addrToHex(items[i].addr);
            obj["environment_id"] = items[i].environment_id;
            obj["device_id"] = items[i].device_id;
            obj["rssi"] = items[i].rssi;
            obj["first_seen_ms"] = items[i].first_seen_ms;
            obj["last_seen_ms"] = items[i].last_seen_ms;
            obj["seen_count"] = items[i].seen_count;
        }

        doc["max"] = MAX_DISCOVERED_BEACONS;
        doc["new_count"] = beaconRegistry.countNew();

        sendJson(request, 200, doc); });
}  
