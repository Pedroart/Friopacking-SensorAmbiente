#pragma once
#include <Arduino.h>
#include "ble_types.h"

#ifndef MAX_DISCOVERED_BEACONS
#define MAX_DISCOVERED_BEACONS 64
#endif

struct DiscoveredBeacon
{
    bool used = false;
    bool isNew = false;
    uint64_t addr = 0;
    uint16_t environment_id = 0;
    uint16_t device_id = 0;
    int8_t rssi = 0;
    uint32_t first_seen_ms = 0;
    uint32_t last_seen_ms = 0;
    uint32_t seen_count = 0;
};

class BeaconRegistry
{
public:
    void begin();
    int find(uint64_t addr) const;
    bool seen(const BeaconDecoded &read);
    int countNew() const;
    void clearNewFlag(int index);

    const DiscoveredBeacon *items() const;

private:
    DiscoveredBeacon list[MAX_DISCOVERED_BEACONS]{};
};
