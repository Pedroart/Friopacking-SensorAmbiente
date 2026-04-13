#pragma once
#include <Arduino.h>
#include "driver/ble_scan.h"
#include "driver/network_manager.h"
#include "driver/storage_nvs.h"
#include "core/deviceConfig.h"
#include "driver/ble_types.h"
#include "driver/slot_manager.h"
#include "driver/beacon_registry.h"

extern StorageNVS storage;
extern SystemConfig sys;
extern FeatureConfig feature;
extern UserConfig users;
extern NetworkConfig network;

struct BootStatus
{
    bool storageReady = false;
    bool filesystemReady = false;
    bool configLoaded = false;
    bool beaconRegistryReady = false;
    bool slotManagerReady = false;
    bool networkApplied = false;
    bool webReady = false;
    bool bleReady = false;
    bool ready = false;
    uint32_t bootCompletedMs = 0;
    String lastError;
};

extern BootStatus bootStatus;

inline void refreshBootReady()
{
    bootStatus.ready =
        bootStatus.storageReady &&
        bootStatus.filesystemReady &&
        bootStatus.configLoaded &&
        bootStatus.slotManagerReady &&
        bootStatus.networkApplied &&
        bootStatus.webReady &&
        bootStatus.bleReady;
}

extern BleProceses advertising;
extern SlotManager slotManager;
extern BeaconRegistry beaconRegistry;
