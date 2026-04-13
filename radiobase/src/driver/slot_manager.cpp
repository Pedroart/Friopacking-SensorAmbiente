#include "slot_manager.h"

static constexpr uint32_t MAP_MAGIC = 0x424D4150; // "BMAP"
static constexpr uint16_t MAP_VERSION = 1;

bool SlotManager::ensureConfigDir() const
{
    File dirTest = LittleFS.open("/config");
    if (dirTest)
    {
        dirTest.close();
        return true;
    }

    return LittleFS.mkdir("/config");
}

void SlotManager::buildMapFileData(MapFileData &fileData, const BeaconMapEntry *entries) const
{
    fileData = {};
    fileData.magic = MAP_MAGIC;
    fileData.version = MAP_VERSION;
    fileData.count = MAX_SLOTS;

    memcpy(fileData.entries, entries, sizeof(fileData.entries));

    fileData.crc = calcCrc32(
        reinterpret_cast<const uint8_t *>(&fileData),
        sizeof(MapFileData) - sizeof(fileData.crc));
}

bool SlotManager::readMapFile(const char *path, BeaconMapEntry *entries) const
{
    if (!LittleFS.exists(path))
    {
        return false;
    }

    File f = LittleFS.open(path, "rb");
    if (!f)
    {
        return false;
    }

    MapFileData fileData{};
    size_t readBytes = f.readBytes(
        reinterpret_cast<char *>(&fileData),
        sizeof(MapFileData));

    f.close();

    if (readBytes != sizeof(MapFileData))
    {
        return false;
    }

    if (fileData.magic != MAP_MAGIC || fileData.version != MAP_VERSION || fileData.count != MAX_SLOTS)
    {
        return false;
    }

    uint32_t expectedCrc = calcCrc32(
        reinterpret_cast<const uint8_t *>(&fileData),
        sizeof(MapFileData) - sizeof(fileData.crc));

    if (expectedCrc != fileData.crc)
    {
        return false;
    }

    memcpy(entries, fileData.entries, sizeof(fileData.entries));
    return true;
}

bool SlotManager::writeMapFile(const char *path, const BeaconMapEntry *entries) const
{
    MapFileData fileData{};
    buildMapFileData(fileData, entries);

    File f = LittleFS.open(path, "wb");
    if (!f)
    {
        return false;
    }

    size_t written = f.write(
        reinterpret_cast<const uint8_t *>(&fileData),
        sizeof(MapFileData));

    f.close();

    return written == sizeof(MapFileData);
}

bool SlotManager::rotateMapFiles() const
{
    if (LittleFS.exists(MAP_FILE_BAK))
    {
        LittleFS.remove(MAP_FILE_BAK);
    }

    if (LittleFS.exists(MAP_FILE))
    {
        if (!LittleFS.rename(MAP_FILE, MAP_FILE_BAK))
        {
            return false;
        }
    }

    if (LittleFS.exists(MAP_FILE))
    {
        LittleFS.remove(MAP_FILE);
    }

    if (LittleFS.rename(MAP_FILE_TMP, MAP_FILE))
    {
        return true;
    }

    if (LittleFS.exists(MAP_FILE_BAK))
    {
        LittleFS.rename(MAP_FILE_BAK, MAP_FILE);
    }

    return false;
}

bool SlotManager::saveMapData(const BeaconMapEntry *entries) const
{
    if (!ensureConfigDir())
    {
        return false;
    }

    if (LittleFS.exists(MAP_FILE_TMP))
    {
        LittleFS.remove(MAP_FILE_TMP);
    }

    if (!writeMapFile(MAP_FILE_TMP, entries))
    {
        LittleFS.remove(MAP_FILE_TMP);
        return false;
    }

    if (!rotateMapFiles())
    {
        LittleFS.remove(MAP_FILE_TMP);
        return false;
    }

    return true;
}

bool SlotManager::begin()
{
    if (mapMutex == nullptr)
    {
        mapMutex = xSemaphoreCreateMutex();
    }

    if (slotsMutex == nullptr)
    {
        slotsMutex = xSemaphoreCreateMutex();
    }

    if (mapMutex == nullptr || slotsMutex == nullptr)
        return false;

    // LittleFS debe estar montado antes de esto en tu sistema
    loadMap();

    return true;
}

bool SlotManager::lockMap(TickType_t timeout)
{
    if (mapMutex == nullptr) return false;
    return xSemaphoreTake(mapMutex, timeout) == pdTRUE;
}

void SlotManager::unlockMap()
{
    if (mapMutex) xSemaphoreGive(mapMutex);
}

bool SlotManager::lockSlots(TickType_t timeout)
{
    if (slotsMutex == nullptr) return false;
    return xSemaphoreTake(slotsMutex, timeout) == pdTRUE;
}

void SlotManager::unlockSlots()
{
    if (slotsMutex) xSemaphoreGive(slotsMutex);
}

int SlotManager::findMappedSlot(uint64_t addr) const
{
    for (int i = 0; i < MAX_SLOTS; ++i)
    {
        if (beaconMap[i].enabled && beaconMap[i].addr == addr)
        {
            return beaconMap[i].slot;
        }
    }
    return -1;
}

void SlotManager::updateSlot(int slot, const BeaconDecoded &read)
{
    if (slot < 0 || slot >= MAX_SLOTS) return;

    slots[slot].used = true;
    slots[slot].addr = read.addr;
    slots[slot].last = read;
    slots[slot].last_seen_ms = millis();
}

bool SlotManager::updateMapped(const BeaconDecoded &read)
{
    if (!lockMap()) return false;
    int slot = findMappedSlot(read.addr);
    unlockMap();

    if (slot < 0) return false;

    if (!lockSlots()) return false;
    updateSlot(slot, read);
    unlockSlots();
    return true;
}

bool SlotManager::updateDirect(const BeaconDecoded &read, uint16_t currentEnv)
{
    if (read.environment_id != currentEnv) return false;
    if (read.device_id >= MAX_SLOTS) return false;

    int slot = static_cast<int>(read.device_id);
    if (!lockSlots()) return false;
    updateSlot(slot, read);
    unlockSlots();
    return true;
}

SlotState *SlotManager::getSlots()
{
    return slots;
}

BeaconMapEntry *SlotManager::getMap()
{
    return beaconMap;
}

uint32_t SlotManager::calcCrc32(const uint8_t *data, size_t len) const
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < len; ++i)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}

bool SlotManager::saveMap() const
{
    BeaconMapEntry snapshot[MAX_SLOTS]{};
    if (!const_cast<SlotManager *>(this)->lockMap()) return false;
    memcpy(snapshot, beaconMap, sizeof(snapshot));
    const_cast<SlotManager *>(this)->unlockMap();

    return saveMapData(snapshot);
}

bool SlotManager::loadMap()
{
    BeaconMapEntry loaded[MAX_SLOTS]{};

    if (readMapFile(MAP_FILE, loaded))
    {
        if (!lockMap()) return false;
        memcpy(beaconMap, loaded, sizeof(beaconMap));
        unlockMap();
        return true;
    }

    if (readMapFile(MAP_FILE_BAK, loaded))
    {
        if (!lockMap()) return false;
        memcpy(beaconMap, loaded, sizeof(beaconMap));
        unlockMap();
        saveMapData(loaded);
        return true;
    }

    if (!lockMap()) return false;
    memset(beaconMap, 0, sizeof(beaconMap));
    unlockMap();
    return false;
}

bool SlotManager::clearMap()
{
    if (lockMap() == false) return false;

    BeaconMapEntry updated[MAX_SLOTS]{};
    unlockMap();

    bool ok = saveMapData(updated);

    if (!lockMap()) return false;
    if (ok)
    {
        memcpy(beaconMap, updated, sizeof(beaconMap));
    }

    unlockMap();
    return ok;
}

bool SlotManager::setMapEntry(int index, uint64_t addr, uint8_t slot, bool enabled)
{
    if (index < 0 || index >= MAX_SLOTS) return false;
    if (slot >= MAX_SLOTS) return false;

    if (!lockMap()) return false;

    BeaconMapEntry updated[MAX_SLOTS]{};
    memcpy(updated, beaconMap, sizeof(updated));
    unlockMap();

    updated[index].enabled = enabled;
    updated[index].addr = addr;
    updated[index].slot = slot;

    bool ok = saveMapData(updated);

    if (!lockMap()) return false;
    if (ok)
    {
        memcpy(beaconMap, updated, sizeof(beaconMap));
    }

    unlockMap();
    return ok;
}
