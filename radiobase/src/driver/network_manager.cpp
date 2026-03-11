#include "network_manager.h"
#include <WiFi.h>

bool applyWifiConfig(const NetworkConfig &Netcfg, const FeatureConfig &Feacfg)
{
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    bool apEnabled = Feacfg.wifiApEnable;
    bool staEnabled = Feacfg.wifiStaEnable;

    if (apEnabled && staEnabled)
    {
        WiFi.mode(WIFI_AP_STA);
    }
    else if (apEnabled)
    {
        WiFi.mode(WIFI_AP);
    }
    else if (staEnabled)
    {
        WiFi.mode(WIFI_STA);
    }
    else
    {
        WiFi.mode(WIFI_OFF);
        return true;
    }

    bool ok = true;

    // -------------------------
    // Configurar AP
    // -------------------------
    if (apEnabled)
    {
        // IP del AP
        if (!WiFi.softAPConfig(Netcfg.ap.net.ip, Netcfg.ap.net.gateway, Netcfg.ap.net.subnet))
        {
            Serial.println("ERROR: no se pudo configurar IP del AP");
            ok = false;
        }

        // Levantar AP
        bool apResult = false;

        if (strlen(Netcfg.ap.password) >= 8)
        {
            apResult = WiFi.softAP(Netcfg.ap.ssid, Netcfg.ap.password, Netcfg.ap.channel);
        }
        else
        {
            // AP abierto si password vacía o < 8
            apResult = WiFi.softAP(Netcfg.ap.ssid, nullptr, Netcfg.ap.channel);
        }

        if (!apResult)
        {
            Serial.println("ERROR: no se pudo iniciar AP");
            ok = false;
        }
        else
        {
            Serial.print("AP iniciado. IP: ");
            Serial.println(WiFi.softAPIP());
        }
    }

    // -------------------------
    // Configurar STA
    // -------------------------
    if (staEnabled)
    {
        if (!Netcfg.sta.net.dhcp)
        {
            if (!WiFi.config(
                    Netcfg.sta.net.ip,
                    Netcfg.sta.net.gateway,
                    Netcfg.sta.net.subnet,
                    Netcfg.sta.net.dns1,
                    Netcfg.sta.net.dns2))
            {
                Serial.println("ERROR: no se pudo configurar IP fija STA");
                ok = false;
            }
        }

        WiFi.begin(Netcfg.sta.ssid, Netcfg.sta.password);

        Serial.print("Conectando STA a: ");
        Serial.println(Netcfg.sta.ssid);
    }

    return ok;
}