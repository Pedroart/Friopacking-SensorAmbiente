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

        request->send(200, "application/json", json);
    });

    server.on("/api/device/info", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "application/json", "{\"device\":\"gateway\"}");
    });
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

        sendJson(request, 200, doc);
    });

    server.on("/api/network/ethernet", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
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

            sendSuccess(request, "Configuracion Ethernet actualizada");
        }
    );

    server.on("/api/network/ap", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
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

            sendSuccess(request, "Configuracion AP actualizada");
        }
    );

    server.on("/api/network/sta", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
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

            sendSuccess(request, "Configuracion STA actualizada");
        }
    );
}