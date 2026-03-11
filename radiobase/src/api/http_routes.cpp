#include "http_routes.h"
#include <WiFi.h>

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