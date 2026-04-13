#pragma once
#include <Arduino.h>
#include "config.h"

struct IpSettings {
    bool dhcp;          // true = automático, false = manual
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
};

struct EthernetConfig {
    IpSettings net;
};

struct WifiApConfig {
    char ssid[32]= {0};
    char password[64]= {0};
    uint8_t channel;
    bool hidden;
    uint8_t max_clients;
    IpSettings net;
};

struct WifiStaConfig {
    char ssid[32]= {0};
    char password[64]= {0};
    IpSettings net;
};

struct NetworkConfig
{
    EthernetConfig eth;
    WifiApConfig ap;
    WifiStaConfig sta;
};