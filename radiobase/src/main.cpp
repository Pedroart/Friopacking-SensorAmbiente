#include <Arduino.h>
#include <bleCallbacks.h>
#include <crypto_lib.h>

QueueHandle_t dataQ = nullptr;

struct SlotState
{
    bool used = false;
    uint8_t device_id = 0;
    
    int16_t tmp_x100 = 0;
    int16_t cpu_x100 = 0;
    uint8_t bat_pct = 0;
    uint8_t flags = 0;

    int8_t rssi_read = 0;
    int8_t rssi_send = 0;

    uint8_t addr[6] = {0};

    uint32_t last_seen_ms = 0;
    
};

static SlotState slots[32];

// Version 1 ////////////

#pragma pack(push, 1)
struct BeaconPayload
{
    uint8_t version_id = 1;
    uint8_t device_id = 0;
    uint8_t flags = 0;
    int16_t TMP117_tem_x100 = 0;
    int16_t CPU_tem_x100 = 0;
    int8_t BAT_x100 = 0;
};
#pragma pack(pop)

struct Flags
{
    bool i2c_fail = false;
    bool bat_low = false;
    bool tmp_fail = false;
};

Flags decodeFlags(uint8_t rawFlags)
{
    Flags f;
    f.i2c_fail = rawFlags & (1 << 0);
    f.bat_low = rawFlags & (1 << 1);
    f.tmp_fail = rawFlags & (1 << 2);
    return f;
}

///////////////////////////

static const uint8_t KEY[16] = {
    0xA3, 0x7F, 0x1C, 0xD9, 0x88, 0x4E, 0x21, 0xB6,
    0x59, 0x02, 0xEF, 0xC4, 0x6A, 0x90, 0x13, 0xDD};

static void updateSlotFromBeacon(SlotState &s, const BeaconPayload &p, const AdvRaw &m)
{
    s.used = true;
    s.device_id = p.device_id;

    s.tmp_x100 = p.TMP117_tem_x100;
    s.cpu_x100 = p.CPU_tem_x100;
    s.bat_pct = (uint8_t)p.BAT_x100;
    s.flags = p.flags;

    s.rssi_read = m.rssi_read;
    s.rssi_send = m.rssi_send;

    memcpy(s.addr, m.addr, 6);

    s.last_seen_ms = millis();
}

void advProcessTask(void *pvParameters)
// TODO - Liberar Memoria
{
    (void)pvParameters;
    AdvRaw m;

    for (;;)
    {
        if (xQueueReceive(advQ, &m, portMAX_DELAY) == pdTRUE)
        {
            Serial.printf("ADV: rssi_read=%d rssi_send=%d addr=%02X:%02X:%02X:%02X:%02X:%02X len=%u payload=",
                          m.rssi_read,
                          m.rssi_send,
                          m.addr[5], m.addr[4], m.addr[3],
                          m.addr[2], m.addr[1], m.addr[0],
                          m.len);

            for (uint8_t i = 0; i < m.len; i++)
            {
                Serial.printf("%02X ", m.payload[i]);
            }
            Serial.println();

            if (m.len < 16)
            {
                Serial.println("AES: payload menor a 16 bytes, no se puede desencriptar");
                continue;
            }

            uint8_t plain[16];

            if (!decrypt_block(m.payload, plain))
            {
                Serial.println("AES: error al desencriptar");
                continue;
            }

            Serial.print("DEC: ");
            for (uint8_t i = 0; i < 16; i++)
            {
                Serial.printf("%02X ", plain[i]);
            }
            Serial.println();

            BeaconPayload p;
            memcpy(&p, plain, sizeof(BeaconPayload));
            
            Flags fl = decodeFlags(p.flags);

            Serial.println("---- Beacon Decodificado ----");
            Serial.printf("version_id      : %u\n", p.version_id);
            Serial.printf("device_id       : %u\n", p.device_id);
            Serial.printf("flags raw       : 0x%02X\n", p.flags);
            Serial.printf("TMP117 temp     : %.2f C\n", p.TMP117_tem_x100 / 100.0f);
            Serial.printf("CPU temp        : %.2f C\n", p.CPU_tem_x100 / 100.0f);
            Serial.printf("BAT raw         : %d\n", p.BAT_x100);

            Serial.println("Flags:");
            Serial.printf("  i2c_fail      : %s\n", fl.i2c_fail ? "true" : "false");
            Serial.printf("  bat_low       : %s\n", fl.bat_low ? "true" : "false");
            Serial.printf("  tmp_fail      : %s\n", fl.tmp_fail ? "true" : "false");
            Serial.println("-----------------------------");

            if (p.device_id < 32)
            {
                updateSlotFromBeacon(slots[p.device_id], p, m);

                Serial.printf("SLOT[%u] actualizado\n", p.device_id);
            }
        }
    }
}

TaskHandle_t advTaskHandle = nullptr;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("MAIN: Sistema iniciado");

    ble_rx_init();

    if (!aes_init_key(KEY))
    {
        Serial.println("AES: error al inicializar clave");
    }

    xTaskCreatePinnedToCore(
        advProcessTask,
        "advProcessTask",
        4096,           // Memoria asignada
        nullptr,        // Ningun parametro
        2,              // Prioridad
        &advTaskHandle, // El puntero de control
        1               // La CPU donde se realiza
    );
}

void loop()
{
    delay(1000);
    Serial.println((unsigned)uxTaskGetStackHighWaterMark(advTaskHandle));
}