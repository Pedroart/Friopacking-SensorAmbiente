#pragma once
#include <bleCallbacks.h>
#include <crypto_lib.h>
#include "config.h"
#include "ble_types.h"
#include "beacon_registry.h"

class BleProceses {
public:
    bool begin();
    BlePipelineStats stats() const;
    void resetStats();

    static void advProcessTask(void *pvParameters);
    static void beaconLogicTask(void *pvParameters);
};
