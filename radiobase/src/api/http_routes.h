#pragma once
#include <ESPAsyncWebServer.h>

void registerHttpRoutes(AsyncWebServer &server);
void registerNetworkRoutes(AsyncWebServer &server);