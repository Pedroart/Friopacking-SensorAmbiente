#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "ble_types.h"

class SlotManager
{
public:
    bool begin();
    bool lockMap(TickType_t timeout = portMAX_DELAY);
    void unlockMap();
    bool lockSlots(TickType_t timeout = portMAX_DELAY);
    void unlockSlots();

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
    static constexpr const char *MAP_FILE_TMP = "/config/beacon_map.tmp";
    static constexpr const char *MAP_FILE_BAK = "/config/beacon_map.bak";

    struct MapFileData
    {
        uint32_t magic;
        uint16_t version;
        uint16_t count;
        BeaconMapEntry entries[MAX_SLOTS];
        uint32_t crc;
    };

    uint32_t calcCrc32(const uint8_t *data, size_t len) const;
    bool ensureConfigDir() const;
    void buildMapFileData(MapFileData &fileData, const BeaconMapEntry *entries) const;
    bool readMapFile(const char *path, BeaconMapEntry *entries) const;
    bool writeMapFile(const char *path, const BeaconMapEntry *entries) const;
    bool rotateMapFiles() const;
    bool saveMapData(const BeaconMapEntry *entries) const;

private:
    SemaphoreHandle_t mapMutex = nullptr;
    SemaphoreHandle_t slotsMutex = nullptr;
    BeaconMapEntry beaconMap[MAX_SLOTS]{};
    SlotState slots[MAX_SLOTS]{};
};

