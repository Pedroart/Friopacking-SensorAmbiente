#pragma once
#include <AsyncWebServer_ESP32_SC_W5500.h>

class WebService
{
public:
    bool begin();
    void loop();
};

void registerHttpPaths(AsyncWebServer &server);

extern WebService webService;
