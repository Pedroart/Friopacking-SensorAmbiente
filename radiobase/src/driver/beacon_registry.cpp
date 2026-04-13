#include "beacon_registry.h"

void BeaconRegistry::begin()
{
    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        list[i] = {};
    }
}

int BeaconRegistry::find(uint64_t addr) const
{
    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        if (list[i].used && list[i].addr == addr)
        {
            return i;
        }
    }
    return -1;
}

bool BeaconRegistry::seen(const BeaconDecoded &read)
{
    uint32_t now = millis();

    int idx = find(read.addr);
    if (idx >= 0)
    {
        list[idx].last_seen_ms = now;
        list[idx].rssi = read.rssi_read;
        list[idx].seen_count++;
        return false; // ya existía
    }

    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        if (!list[i].used)
        {
            list[i].used = true;
            list[i].isNew = true;
            list[i].addr = read.addr;
            list[i].environment_id = read.environment_id;
            list[i].device_id = read.device_id;
            list[i].rssi = read.rssi_read;
            list[i].first_seen_ms = now;
            list[i].last_seen_ms = now;
            list[i].seen_count = 1;
            return true; // nuevo dispositivo
        }
    }

    return false; // no había espacio
}

int BeaconRegistry::countNew() const
{
    int n = 0;
    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        if (list[i].used && list[i].isNew) n++;
    }
    return n;
}

void BeaconRegistry::clearNewFlag(int index)
{
    if (index < 0 || index >= MAX_DISCOVERED_BEACONS) return;
    if (list[index].used) list[index].isNew = false;
}

const DiscoveredBeacon *BeaconRegistry::items() const
{
    return list;
}