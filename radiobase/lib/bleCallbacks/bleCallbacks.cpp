#include "bleCallbacks.h"
#include "ble_pipeline_stats.h"

QueueHandle_t advQ = nullptr;
NimBLEScan *scan = nullptr;
static uint32_t lastAdvDropLogMs = 0;
static constexpr uint32_t BLE_DROP_LOG_INTERVAL_MS = 5000;

class ScanCallbacks : public NimBLEScanCallbacks
{
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
    {

        // Data Envia
        // ADV: rssi=-36
        // addr=D1:75:09:64:21:40
        // len=26
        // payload=02 01 06 02 0A 00 13 FF F5 10 8C E7 B1 A9 FC 50 69 65 83 4A 18 B5 9D 7B BD 98

        const uint8_t *p = advertisedDevice->getPayload().data();
        const size_t plen = advertisedDevice->getPayload().size();

        // Formato esperado:
        // 02 01 06 02 0A XX 13 FF F5 10 <data...>
        if (plen < 10)
            return;

        // filtro fijo por posiciones
        if (p[7] != 0xFF)
            return;
        if (p[8] != 0x10 || p[9] != 0xF5)
            return;

        AdvRaw m{};

        m.rssi_read = advertisedDevice->getRSSI();
        m.rssi_send = (int8_t)p[5];
        m.rx_ms = millis();

        NimBLEAddress a = advertisedDevice->getAddress();
        memcpy(m.addr, a.getVal(), 6);
/*
        Serial.printf("BLE_SCAN MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                      m.addr[0], m.addr[1], m.addr[2], m.addr[3], m.addr[4], m.addr[5]);
*/
        const size_t data_start = 10;
        size_t data_len = plen - data_start;
        if (data_len > sizeof(m.payload))
            data_len = sizeof(m.payload);

        m.len = (uint8_t)data_len;
        memcpy(m.payload, p + data_start, data_len);

        BaseType_t ok = xQueueSend(advQ, &m, 0);
        if (ok != pdTRUE)
        {
            bleStatsRecordAdvDropped(static_cast<uint32_t>(uxQueueMessagesWaiting(advQ)));
            uint32_t now = millis();
            if ((now - lastAdvDropLogMs) >= BLE_DROP_LOG_INTERVAL_MS)
            {
                lastAdvDropLogMs = now;
                BlePipelineStats snapshot = bleStatsSnapshot();
                Serial.printf(
                    "BLE advQ drops adv=%lu data=%lu decrypt=%lu advDepth=%lu/%lu dataDepth=%lu/%lu\n",
                    static_cast<unsigned long>(snapshot.adv_dropped),
                    static_cast<unsigned long>(snapshot.data_dropped),
                    static_cast<unsigned long>(snapshot.adv_decrypt_fail),
                    static_cast<unsigned long>(snapshot.current_adv_depth),
                    static_cast<unsigned long>(snapshot.max_adv_depth),
                    static_cast<unsigned long>(snapshot.current_data_depth),
                    static_cast<unsigned long>(snapshot.max_data_depth));
            }
        }
        else
        {
            bleStatsRecordAdvReceived(static_cast<uint32_t>(uxQueueMessagesWaiting(advQ)));
        }
    }
};

void ble_rx_init()
{
    Serial.printf("BLE: Initializing BLE...");
    NimBLEDevice::init("");

    if (!advQ)
    {
        const int ndevice = 16;
        const int nsed = 3;

        advQ = xQueueCreate(ndevice * nsed, sizeof(AdvRaw));
        Serial.printf("BLE: Queue created");
    }

    scan = NimBLEDevice::getScan();
    Serial.printf("BLE: Scan object created");

    scan->setScanCallbacks(new ScanCallbacks(), true);
    scan->setActiveScan(false);
    scan->setInterval(160);
    scan->setWindow(120);
    scan->setMaxResults(0);

    scan->start(0, true, false);
    Serial.printf("BLE: Scan started");
};
