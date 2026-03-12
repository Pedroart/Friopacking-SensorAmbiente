#pragma once
#include <ESPAsyncWebServer.h>

class WebService
{
public:
    bool begin();
    void loop();
};

void registerHttpPaths(AsyncWebServer &server);

extern WebService webService;
