#include "beacon_registry.h"

void BeaconRegistry::begin()
{
    if (mutex == nullptr)
    {
        mutex = xSemaphoreCreateMutex();
    }

    if (mutex == nullptr) return;
    if (!lock()) return;

    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        list[i] = {};
    }

    unlock();
}

bool BeaconRegistry::lock(TickType_t timeout) const
{
    if (mutex == nullptr) return false;
    return xSemaphoreTake(mutex, timeout) == pdTRUE;
}

void BeaconRegistry::unlock() const
{
    if (mutex) xSemaphoreGive(mutex);
}

int BeaconRegistry::find(uint64_t addr) const
{
    if (!lock()) return -1;

    int found = -1;

    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        if (list[i].used && list[i].addr == addr)
        {
            found = i;
            break;
        }
    }

    unlock();
    return found;
}

bool BeaconRegistry::seen(const BeaconDecoded &read)
{
    uint32_t now = millis();

    if (!lock()) return false;

    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        if (list[i].used && list[i].addr == read.addr)
        {
            list[i].last_seen_ms = now;
            list[i].rssi = read.rssi_read;
            list[i].seen_count++;
            unlock();
            return false;
        }
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
            unlock();
            return true;
        }
    }

    unlock();
    return false;
}

int BeaconRegistry::countNew() const
{
    if (!lock()) return 0;

    int n = 0;
    for (int i = 0; i < MAX_DISCOVERED_BEACONS; ++i)
    {
        if (list[i].used && list[i].isNew) n++;
    }

    unlock();
    return n;
}

void BeaconRegistry::clearNewFlag(int index)
{
    if (!lock()) return;

    if (index < 0 || index >= MAX_DISCOVERED_BEACONS)
    {
        unlock();
        return;
    }

    if (list[index].used)
    {
        list[index].isNew = false;
    }

    unlock();
}

bool BeaconRegistry::snapshot(DiscoveredBeacon *out, size_t count) const
{
    if (out == nullptr || count < MAX_DISCOVERED_BEACONS)
    {
        return false;
    }

    if (!lock()) return false;
    memcpy(out, list, sizeof(list));
    unlock();
    return true;
}
