#include "slot_manager.h"

static constexpr uint32_t MAP_MAGIC = 0x424D4150; // "BMAP"
static constexpr uint16_t MAP_VERSION = 1;

bool SlotManager::begin()
{
    if (mutex == nullptr)
    {
        mutex = xSemaphoreCreateMutex();
    }

    if (mutex == nullptr)
        return false;

    // LittleFS debe estar montado antes de esto en tu sistema
    loadMap();

    return true;
}

bool SlotManager::lock(TickType_t timeout)
{
    if (mutex == nullptr) return false;
    return xSemaphoreTake(mutex, timeout) == pdTRUE;
}

void SlotManager::unlock()
{
    if (mutex) xSemaphoreGive(mutex);
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
    int slot = findMappedSlot(read.addr);
    if (slot < 0) return false;

    updateSlot(slot, read);
    return true;
}

bool SlotManager::updateDirect(const BeaconDecoded &read, uint16_t currentEnv)
{
    if (read.environment_id != currentEnv) return false;
    if (read.device_id >= MAX_SLOTS) return false;

    int slot = static_cast<int>(read.device_id);
    updateSlot(slot, read);
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
    File dirTest = LittleFS.open("/config");
    if (!dirTest)
    {
        LittleFS.mkdir("/config");
    }
    else
    {
        dirTest.close();
    }

    MapFileData fileData{};
    fileData.magic = MAP_MAGIC;
    fileData.version = MAP_VERSION;
    fileData.count = MAX_SLOTS;

    memcpy(fileData.entries, beaconMap, sizeof(beaconMap));

    fileData.crc = calcCrc32(
        reinterpret_cast<const uint8_t *>(&fileData),
        sizeof(MapFileData) - sizeof(fileData.crc));

    File f = LittleFS.open(MAP_FILE, "wb");
    if (!f)
        return false;

    size_t written = f.write(
        reinterpret_cast<const uint8_t *>(&fileData),
        sizeof(MapFileData));

    f.close();

    return written == sizeof(MapFileData);
}

bool SlotManager::loadMap()
{
    if (!LittleFS.exists(MAP_FILE))
    {
        memset(beaconMap, 0, sizeof(beaconMap));
        return false;
    }

    File f = LittleFS.open(MAP_FILE, "rb");
    if (!f)
    {
        memset(beaconMap, 0, sizeof(beaconMap));
        return false;
    }

    MapFileData fileData{};
    size_t readBytes = f.readBytes(
        reinterpret_cast<char *>(&fileData),
        sizeof(MapFileData));

    f.close();

    if (readBytes != sizeof(MapFileData))
    {
        memset(beaconMap, 0, sizeof(beaconMap));
        return false;
    }

    if (fileData.magic != MAP_MAGIC || fileData.version != MAP_VERSION || fileData.count != MAX_SLOTS)
    {
        memset(beaconMap, 0, sizeof(beaconMap));
        return false;
    }

    uint32_t expectedCrc = calcCrc32(
        reinterpret_cast<const uint8_t *>(&fileData),
        sizeof(MapFileData) - sizeof(fileData.crc));

    if (expectedCrc != fileData.crc)
    {
        memset(beaconMap, 0, sizeof(beaconMap));
        return false;
    }

    memcpy(beaconMap, fileData.entries, sizeof(beaconMap));
    return true;
}

bool SlotManager::clearMap()
{
    if (lock() == false) return false;

    memset(beaconMap, 0, sizeof(beaconMap));
    bool ok = saveMap();

    unlock();
    return ok;
}

bool SlotManager::setMapEntry(int index, uint64_t addr, uint8_t slot, bool enabled)
{
    if (index < 0 || index >= MAX_SLOTS) return false;
    if (slot >= MAX_SLOTS) return false;

    if (!lock()) return false;

    beaconMap[index].enabled = enabled;
    beaconMap[index].addr = addr;
    beaconMap[index].slot = slot;

    bool ok = saveMap();

    unlock();
    return ok;
}