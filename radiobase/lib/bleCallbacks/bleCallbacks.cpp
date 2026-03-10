#include "bleCallbacks.h"

QueueHandle_t advQ = nullptr;
NimBLEScan *scan = nullptr;

class ScanCallbacks : public NimBLEScanCallbacks
{
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
    {

        // Data Envia
        // ADV: rssi=-36
        // addr=D1:75:09:64:21:40
        // len=26
        // payload=02 01 06 02 0A 00 13 FF F5 10 8C E7 B1 A9 FC 50 69 65 83 4A 18 B5 9D 7B BD 98

        const uint8_t* p = advertisedDevice->getPayload().data();
        const size_t plen = advertisedDevice->getPayload().size();

        // Formato esperado:
        // 02 01 06 02 0A XX 13 FF F5 10 <data...>
        if (plen < 10) return;

        // filtro fijo por posiciones
        if (p[7] != 0xFF) return;
        if (p[8] != 0x10 || p[9] != 0xF5) return;

        AdvRaw m{};

        m.rssi_read = advertisedDevice->getRSSI();
        m.rssi_send = (int8_t)p[5];

        NimBLEAddress a = advertisedDevice->getAddress();
        memcpy(m.addr, a.getVal(), 6);
        
        const size_t data_start = 10;
        size_t data_len = plen - data_start;
        if (data_len > sizeof(m.payload)) data_len = sizeof(m.payload);

        m.len = (uint8_t)data_len;
        memcpy(m.payload, p + data_start, data_len);

        (void)xQueueSend(advQ, &m, 0);
    }
};

void ble_rx_init()
{
    Serial.printf("BLE: Initializing BLE...");
    NimBLEDevice::init("");

    scan = NimBLEDevice::getScan();
    Serial.printf("BLE: Scan object created");

    scan->setScanCallbacks(new ScanCallbacks(), false);
    scan->setActiveScan(false);
    scan->setInterval(160);
    scan->setWindow(120);
    scan->setMaxResults(0);

    scan->start(0, true, false);
    Serial.printf("BLE: Scan started");

    if (!advQ)
    {
        const int ndevice = 16;
        const int nsed = 3;

        advQ = xQueueCreate(ndevice*nsed, sizeof(AdvRaw));
        Serial.printf("BLE: Queue created");
    }
};
