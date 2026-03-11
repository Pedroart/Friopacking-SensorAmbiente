#pragma once
#include <ESPAsyncWebServer.h>

void registerWsRoutes(AsyncWebSocket &ws);
void wsBroadcastText(const char *msg);