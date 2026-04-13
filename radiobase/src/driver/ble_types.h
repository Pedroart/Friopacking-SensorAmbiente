#pragma once

#include "config.h"

extern QueueHandle_t dataQ;
extern SemaphoreHandle_t slotsMutex;
extern TaskHandle_t advTaskHandle;

/* Estructura de Paquete de llegada

  uint8_t version_id = 1;
  uint8_t environment_id = 1;
  uint8_t device_id = 0;
  uint8_t flags = 0;
  int16_t TMP117_tem_x100 = 0;
  int16_t CPU_tem_x100 = 0;
  int8_t BAT_x100 = 0;

*/

#pragma pack(push,1)
struct BeaconBody
{
    uint8_t version_id = 0;
    uint8_t environment_id = 0;
    uint8_t device_id = 0;

    union
    {
        uint8_t raw;

        struct
        {
            uint8_t i2c_fail : 1;
            uint8_t bat_low  : 1;
            uint8_t tmp_fail : 1;
            uint8_t reserved : 5;
        };
    } flags;

    int16_t tmp_x100 = 0;
    int16_t cpu_x100 = 0;
    int8_t bat_pct = 0;
};
#pragma pack(pop)

#pragma pack(push,1)
struct BeaconDecoded
{
    uint8_t version_id = 0;
    uint8_t environment_id = 0;
    uint8_t device_id = 0;
    uint8_t flags = 0;
    int16_t tmp_x100 = 0;
    int16_t cpu_x100 = 0;
    int8_t bat_pct = 0;

    int8_t rssi_read = 0;
    int8_t rssi_send = 0;
    uint64_t addr = 0;

    uint32_t rx_ms = 0;
};
#pragma pack(pop)

struct BeaconMapEntry
{
    uint64_t addr;
    uint8_t slot;
    bool enabled;
};

struct SlotState
{
    bool used;
    uint64_t addr;
    BeaconDecoded last;
    uint32_t last_seen_ms;
};
