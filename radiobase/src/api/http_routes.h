#pragma once
#include <AsyncWebServer_ESP32_SC_W5500.h>

void registerHttpRoutes(AsyncWebServer &server);
void registerNetworkRoutes(AsyncWebServer &server);
void registerBeaconRoutes(AsyncWebServer &server);
void registerFeatureRoutes(AsyncWebServer &server);