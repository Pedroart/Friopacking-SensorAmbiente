#pragma once
#include <ArduinoJson.h>
#include <AsyncWebServer_ESP32_SC_W5500.h>
#include "core/networkConfig.h"

bool parseIpField(JsonVariant src, IPAddress &out);
void sendJson(AsyncWebServerRequest *request, int code, JsonDocument &doc);
void ipToJson(JsonObject obj, const IpSettings &net);
bool jsonToIpSettings(JsonObject obj, IpSettings &net, String &error);
void sendError(AsyncWebServerRequest *request, int code, const String &message);
void sendSuccess(AsyncWebServerRequest *request, const String &message);
JsonObject createResponse(JsonDocument &doc, bool success, const String &message = "");
void sendData(AsyncWebServerRequest *request, int code, JsonDocument &doc, const String &message = "");


String uint64ToHex(uint64_t value);
uint64_t hexToUint64(const String &hex);

String addrToHex(uint64_t value);
