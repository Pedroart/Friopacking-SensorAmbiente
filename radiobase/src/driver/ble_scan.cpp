#include "ble_scan.h"
#include "slot_manager.h"
#include "core/appState.h"

QueueHandle_t dataQ = nullptr;
SemaphoreHandle_t slotsMutex = nullptr;
TaskHandle_t advTaskHandle = nullptr;
TaskHandle_t beaconLogicTaskHandle = nullptr;

static const uint8_t KEY[16] = {
    0xA3, 0x7F, 0x1C, 0xD9, 0x88, 0x4E, 0x21, 0xB6,
    0x59, 0x02, 0xEF, 0xC4, 0x6A, 0x90, 0x13, 0xDD};

void BleProceses::begin()
{
    aes_init_key(KEY);
    ble_rx_init();

    if (dataQ == nullptr)
    {
        dataQ = xQueueCreate(ADV_DATA_QUEUE_LEN, sizeof(BeaconDecoded));
    }
    
    if (dataQ == nullptr)
    {
        Serial.println("BleProceses: No se pudo iniciar dataQ");
        return;
    }

    BaseType_t ok = xTaskCreatePinnedToCore(
        advProcessTask,
        "advProcessTask",
        4096,
        nullptr,
        2,
        &advTaskHandle,
        1);

    if (ok != pdPASS)
    {
        // log de error si quieres
        Serial.println("BleProceses: No se pudo iniciar advProcessTask");
        advTaskHandle = nullptr;
    }

    BaseType_t ok2 = xTaskCreatePinnedToCore(
        beaconLogicTask,
        "beaconLogicTask",
        4096,
        nullptr,
        2,
        &beaconLogicTaskHandle,
        1);

    if (ok2 != pdPASS)
    {
        Serial.println("BleProceses: No se pudo iniciar beaconLogicTask");
        return;
    }

    Serial.println("BleProceses: Inicializado");
}

void BleProceses::advProcessTask(void *pvParameters)
{
    (void)pvParameters;
    AdvRaw m;
    Serial.println("advProcessTask: Iniciado");

    for (;;)
    {
        if (xQueueReceive(advQ, &m, portMAX_DELAY) == pdTRUE)
        {
            Serial.printf("advProcessTask: recibido RSSI=%d len=%u\n",
                          m.rssi_read, m.len);
            uint8_t plain[16];

            // Error en decrpyt bloque skip
            if (!decrypt_block(m.payload, plain))
            {
                Serial.print("advProcessTask: Problema decrypt");
                continue;
            }

            BeaconDecoded read{};

            memcpy(&read, plain, sizeof(BeaconBody));

            // if (read.version_id == 1) return;

            // Optimizacion
            // memcpy(read.addr, m.addr, sizeof(read.addr));
            for (int i = 0; i < 6; ++i)
            {
                read.addr = (read.addr << 8) | m.addr[i];
            }

            Serial.printf("advProcessTask: %02X:%02X:%02X:%02X:%02X:%02X",
                          m.addr[0], m.addr[1], m.addr[2], m.addr[3], m.addr[4], m.addr[5]);

            read.rssi_read = m.rssi_read;
            read.rssi_send = m.rssi_send;
            read.rx_ms = m.rx_ms;

            if (dataQ)
            {
                xQueueSend(dataQ, &read, 0);
            }
        }
    }
}

void BleProceses::beaconLogicTask(void *pvParameters)
{
    (void)pvParameters;
    BeaconDecoded read;

    for (;;)
    {
        if (xQueueReceive(dataQ, &read, portMAX_DELAY) != pdTRUE)
            continue;

        bool handled = false;

        if (slotManager.lock(portMAX_DELAY))
        {
            handled = slotManager.updateMapped(read);

            if (!handled)
            {
                uint16_t currentEnv = static_cast<uint16_t>(sys.ambiente);
                handled = slotManager.updateDirect(read, currentEnv);
            }

            slotManager.unlock();
        }

        if (!handled)
        {
            bool isNew = beaconRegistry.seen(read);

            if (isNew)
            {
                Serial.printf("Nuevo beacon detectado: addr=%llX env=%u dev=%u rssi=%d\n",
                              read.addr,
                              read.environment_id,
                              read.device_id,
                              read.rssi_read);
            }
        }
    }
}
