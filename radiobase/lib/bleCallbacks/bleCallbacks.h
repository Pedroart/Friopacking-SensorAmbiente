#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <NimBLEDevice.h>

struct AdvRaw
{
    int8_t rssi_read;
    int8_t rssi_send;
    uint8_t addr[6];
    uint8_t len;
    uint8_t payload[16];
    uint32_t rx_ms;
};

extern QueueHandle_t advQ;
extern NimBLEScan* scan;

void ble_rx_init();