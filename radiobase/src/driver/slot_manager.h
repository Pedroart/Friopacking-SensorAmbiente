#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "ble_types.h"

class SlotManager
{
public:
    bool begin();
    bool lock(TickType_t timeout = portMAX_DELAY);
    void unlock();

    int findMappedSlot(uint64_t addr) const;
    bool updateMapped(const BeaconDecoded &read);
    bool updateDirect(const BeaconDecoded &read, uint16_t currentEnv);
    void updateSlot(int slot, const BeaconDecoded &read);

    SlotState *getSlots();
    BeaconMapEntry *getMap();

    bool loadMap();
    bool saveMap() const;
    bool clearMap();
    bool setMapEntry(int index, uint64_t addr, uint8_t slot, bool enabled = true);

private:
    static constexpr const char *MAP_FILE = "/config/beacon_map.bin";

    struct MapFileData
    {
        uint32_t magic;
        uint16_t version;
        uint16_t count;
        BeaconMapEntry entries[MAX_SLOTS];
        uint32_t crc;
    };

    uint32_t calcCrc32(const uint8_t *data, size_t len) const;

private:
    SemaphoreHandle_t mutex = nullptr;
    BeaconMapEntry beaconMap[MAX_SLOTS]{};
    SlotState slots[MAX_SLOTS]{};
};

