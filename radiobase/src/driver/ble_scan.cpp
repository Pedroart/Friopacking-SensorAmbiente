#include "ble_scan.h"
#include "slot_manager.h"
#include "core/appState.h"

QueueHandle_t dataQ = nullptr;
TaskHandle_t advTaskHandle = nullptr;
TaskHandle_t beaconLogicTaskHandle = nullptr;
BlePipelineStats bleStats{};

static portMUX_TYPE bleStatsMux = portMUX_INITIALIZER_UNLOCKED;
static uint32_t lastDataDropLogMs = 0;

static const uint8_t KEY[16] = {
    0xA3, 0x7F, 0x1C, 0xD9, 0x88, 0x4E, 0x21, 0xB6,
    0x59, 0x02, 0xEF, 0xC4, 0x6A, 0x90, 0x13, 0xDD};

static constexpr uint32_t BLE_DROP_LOG_INTERVAL_MS = 5000;

static void updateMax(uint32_t &currentMax, uint32_t value)
{
    if (value > currentMax)
    {
        currentMax = value;
    }
}

void bleStatsReset()
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats = {};
    portEXIT_CRITICAL(&bleStatsMux);
}

BlePipelineStats bleStatsSnapshot()
{
    portENTER_CRITICAL(&bleStatsMux);
    BlePipelineStats snapshot = bleStats;
    portEXIT_CRITICAL(&bleStatsMux);
    return snapshot;
}

void bleStatsRecordAdvReceived(uint32_t depth)
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.adv_received++;
    bleStats.current_adv_depth = depth;
    updateMax(bleStats.max_adv_depth, depth);
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordAdvDropped(uint32_t depth)
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.adv_dropped++;
    bleStats.current_adv_depth = depth;
    updateMax(bleStats.max_adv_depth, depth);
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordAdvDecryptFail()
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.adv_decrypt_fail++;
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordDataEnqueued(uint32_t depth)
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.data_enqueued++;
    bleStats.current_data_depth = depth;
    updateMax(bleStats.max_data_depth, depth);
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordDataDropped(uint32_t depth)
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.data_dropped++;
    bleStats.current_data_depth = depth;
    updateMax(bleStats.max_data_depth, depth);
    portEXIT_CRITICAL(&bleStatsMux);
}

static void logDropSummaryThrottled(const char *tag, uint32_t &lastLogMs)
{
    uint32_t now = millis();
    if ((now - lastLogMs) < BLE_DROP_LOG_INTERVAL_MS)
    {
        return;
    }

    lastLogMs = now;
    BlePipelineStats snapshot = bleStatsSnapshot();
    Serial.printf(
        "%s drops adv=%lu data=%lu decrypt=%lu advDepth=%lu/%lu dataDepth=%lu/%lu e2eMax=%lums\n",
        tag,
        static_cast<unsigned long>(snapshot.adv_dropped),
        static_cast<unsigned long>(snapshot.data_dropped),
        static_cast<unsigned long>(snapshot.adv_decrypt_fail),
        static_cast<unsigned long>(snapshot.current_adv_depth),
        static_cast<unsigned long>(snapshot.max_adv_depth),
        static_cast<unsigned long>(snapshot.current_data_depth),
        static_cast<unsigned long>(snapshot.max_data_depth),
        static_cast<unsigned long>(snapshot.max_end_to_end_ms));
}

void bleStatsRecordProcessed(uint32_t endToEndMs)
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.data_processed++;
    updateMax(bleStats.max_end_to_end_ms, endToEndMs);
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordMappedUpdate()
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.mapped_updates++;
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordDirectUpdate()
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.direct_updates++;
    portEXIT_CRITICAL(&bleStatsMux);
}

void bleStatsRecordRegistryUpdate(bool isNew)
{
    portENTER_CRITICAL(&bleStatsMux);
    bleStats.registry_updates++;
    if (isNew)
    {
        bleStats.registry_new++;
    }
    portEXIT_CRITICAL(&bleStatsMux);
}

BlePipelineStats BleProceses::stats() const
{
    return bleStatsSnapshot();
}

void BleProceses::resetStats()
{
    bleStatsReset();
}

bool BleProceses::begin()
{
    aes_init_key(KEY);
    ble_rx_init();
    bleStatsReset();

    if (dataQ == nullptr)
    {
        dataQ = xQueueCreate(ADV_DATA_QUEUE_LEN, sizeof(BeaconDecoded));
    }
    
    if (dataQ == nullptr)
    {
        Serial.println("BleProceses: No se pudo iniciar dataQ");
        return false;
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
        Serial.println("BleProceses: No se pudo iniciar advProcessTask");
        advTaskHandle = nullptr;
        return false;
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
        beaconLogicTaskHandle = nullptr;
        return false;
    }

    Serial.println("BleProceses: Inicializado");
    return true;
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
            uint8_t plain[16];

            // Error en decrpyt bloque skip
            if (!decrypt_block(m.payload, plain))
            {
                bleStatsRecordAdvDecryptFail();
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

            read.rssi_read = m.rssi_read;
            read.rssi_send = m.rssi_send;
            read.rx_ms = m.rx_ms;

            if (dataQ)
            {
                BaseType_t ok = xQueueSend(dataQ, &read, 0);
                uint32_t depth = static_cast<uint32_t>(uxQueueMessagesWaiting(dataQ));

                if (ok == pdTRUE)
                {
                    bleStatsRecordDataEnqueued(depth);
                }
                else
                {
                    bleStatsRecordDataDropped(depth);
                    logDropSummaryThrottled("BLE dataQ", lastDataDropLogMs);
                }
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
        bool updatedMapped = false;
        bool updatedDirect = false;

        handled = slotManager.updateMapped(read);
        updatedMapped = handled;

        if (!handled)
        {
            uint16_t currentEnv = static_cast<uint16_t>(sys.ambiente);
            handled = slotManager.updateDirect(read, currentEnv);
            updatedDirect = handled;
        }

        if (updatedMapped)
        {
            bleStatsRecordMappedUpdate();
        }

        if (updatedDirect)
        {
            bleStatsRecordDirectUpdate();
        }

        if (!handled)
        {
            bool isNew = beaconRegistry.seen(read);
            bleStatsRecordRegistryUpdate(isNew);
        }

        bleStatsRecordProcessed(millis() - read.rx_ms);
    }
}
