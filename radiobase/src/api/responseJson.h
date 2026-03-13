#pragma once
#include <ESPAsyncWebServer.h>
#include "core/networkConfig.h"

bool parseIpField(JsonVariant src, IPAddress &out);
void sendJson(AsyncWebServerRequest *request, int code, JsonDocument &doc);
void ipToJson(JsonObject obj, const IpSettings &net);
bool jsonToIpSettings(JsonObject obj, IpSettings &net, String &error);
void sendError(AsyncWebServerRequest *request, int code, const String &message);
void sendSuccess(AsyncWebServerRequest *request, const String &message);