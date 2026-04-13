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
extern BleProceses advertising;
extern SlotManager slotManager;
extern BeaconRegistry beaconRegistry;