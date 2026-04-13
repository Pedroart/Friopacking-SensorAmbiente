#pragma once
#include <AsyncWebServer_ESP32_SC_W5500.h>

void registerWsRoutes(AsyncWebSocket &ws);
void wsBroadcastText(const char *msg);