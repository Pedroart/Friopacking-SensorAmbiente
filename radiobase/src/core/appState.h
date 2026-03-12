#pragma once
#include <Arduino.h>
#include "driver/network_manager.h"
#include "driver/storage_nvs.h"
#include "core/deviceConfig.h"

extern StorageNVS storage;
extern SystemConfig sys;
extern FeatureConfig feature;
extern UserConfig users;
extern NetworkConfig network;