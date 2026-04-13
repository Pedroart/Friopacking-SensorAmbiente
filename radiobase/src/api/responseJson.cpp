#include "responseJson.h"
#include <ArduinoJson.h>
#include "core/appState.h"

void sendJson(AsyncWebServerRequest *request, int code, JsonDocument &doc)
{
    String out;
    serializeJson(doc, out);
    request->send(code, "application/json", out);
}

JsonObject createResponse(JsonDocument &doc, bool success, const String &message)
{
    doc["success"] = success;
    if (!message.isEmpty())
    {
        doc["message"] = message;
    }

    return doc["data"].to<JsonObject>();
}

void sendData(AsyncWebServerRequest *request, int code, JsonDocument &doc, const String &message)
{
    JsonDocument outDoc;
    JsonObject data = createResponse(outDoc, code >= 200 && code < 300, message);
    data.set(doc.as<JsonVariantConst>());
    sendJson(request, code, outDoc);
}

void ipToJson(JsonObject obj, const IpSettings &net)
{
    obj["dhcp"] = net.dhcp;
    obj["ip"] = net.ip.toString();
    obj["gateway"] = net.gateway.toString();
    obj["subnet"] = net.subnet.toString();
    obj["dns1"] = net.dns1.toString();
    obj["dns2"] = net.dns2.toString();
}


bool parseIpField(JsonVariant src, IPAddress &out)
{
    const char *value = src | "";
    if (!out.fromString(value))
    {
        return false;
    }
    return true;
}

bool jsonToIpSettings(JsonObject obj, IpSettings &net, String &error)
{
    net.dhcp = obj["dhcp"] | net.dhcp;

    if (!net.dhcp)
    {
        if (obj["ip"].isNull() || !parseIpField(obj["ip"], net.ip))
        {
            error = "ip invalida";
            return false;
        }

        if (obj["gateway"].isNull() || !parseIpField(obj["gateway"], net.gateway))
        {
            error = "gateway invalida";
            return false;
        }

        if (obj["subnet"].isNull() || !parseIpField(obj["subnet"], net.subnet))
        {
            error = "subnet invalida";
            return false;
        }

        if (!obj["dns1"].isNull() && !parseIpField(obj["dns1"], net.dns1))
        {
            error = "dns1 invalida";
            return false;
        }

        if (!obj["dns2"].isNull() && !parseIpField(obj["dns2"], net.dns2))
        {
            error = "dns2 invalida";
            return false;
        }
    }

    return true;
}

void sendError(AsyncWebServerRequest *request, int code, const String &message)
{
    JsonDocument doc;
    doc["success"] = false;
    doc["message"] = message;
    JsonObject error = doc["error"].to<JsonObject>();
    error["code"] = message;
    sendJson(request, code, doc);
}

void sendSuccess(AsyncWebServerRequest *request, const String &message)
{
    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = message;
    doc["data"].to<JsonObject>();
    sendJson(request, 200, doc);
}

String uint64ToHex(uint64_t value)
{
    char buf[17];
    sprintf(buf, "%012llX", value); // 6 bytes MAC
    return String(buf);
}

uint64_t hexToUint64(const String &hex)
{
    return strtoull(hex.c_str(), nullptr, 16);
}

String addrToHex(uint64_t value)
{
    char buf[17];
    sprintf(buf, "%012llX", value);
    return String(buf);
}
