#include "network_manager.h"
#include <WiFi.h>
#include <SPI.h>
#include <AsyncWebServer_ESP32_SC_W5500.h>
#include "config.h"

bool applyNetworkConfig(const NetworkConfig &Netcfg, const FeatureConfig &Feacfg)
{
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    bool apEnabled = Feacfg.wifiApEnable;
    bool staEnabled = Feacfg.wifiStaEnable;
    bool ethEnabled = Feacfg.ethernetEnable;

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

    // -------------------------
    // Configurar Ethernet
    // -------------------------
    if (ethEnabled)
    {

        pinMode(ETH_RST, OUTPUT);
        digitalWrite(ETH_RST, LOW);
        delay(60);
        digitalWrite(ETH_RST, HIGH);
        delay(160);

        ESP32_W5500_onEvent();

        uint64_t chipid = ESP.getEfuseMac();

        uint8_t ethMac[6];
        ethMac[0] = (chipid >> 40) & 0xFF;
        ethMac[1] = (chipid >> 32) & 0xFF;
        ethMac[2] = (chipid >> 24) & 0xFF;
        ethMac[3] = (chipid >> 16) & 0xFF;
        ethMac[4] = (chipid >> 8) & 0xFF;
        ethMac[5] = chipid & 0xFF;

        bool ok = ETH.begin(
            ETH_MISO,
            ETH_MOSI,
            ETH_SCK,
            ETH_CS,
            ETH_INT,
            ETH_SPI_CLOCK_MHZ,
            ETH_SPI_HOST,
            ethMac);
    }

    return ok;
}