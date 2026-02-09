#include "config.h"
#include "deepsleep.h"
#include "buttonmode.h"
#include "kvstore.h"
#include "tmp117.h"
#include "vinadc.h"
#include "adv_lib.h"
#include "crypto_lib.h"
#include "config_mode.h"
enum class BootMode : uint8_t
{
  beacon = 0,
  config = 1
};

struct BeaconPayload
{
  // HEAD
  int16_t company = 0xF510; // Friopacking S.L.
  // BODY
  uint8_t device_id = 0;
  uint8_t version_id = 1;
  uint8_t flags = 0;
  int16_t TMP117_tem_x100 = 0;
  int16_t CPU_tem_x100 = 0;
  int8_t BAT_x100 = 0;
};

static void beacon_to_head_body(const BeaconPayload &p,
                                uint8_t *head, size_t head_len,
                                uint8_t *body, size_t body_len)
{
  // ---- HEAD (2 bytes) ----
  // company = 0xF510 → [F5, 10]
  if (head && head_len >= 2)
  {
    head[0] = (uint8_t)((p.company >> 8) & 0xFF); // MSB
    head[1] = (uint8_t)(p.company & 0xFF);        // LSB
  }

  // ---- BODY (8 bytes) ----
  if (body && body_len >= 8)
  {
    size_t i = 0;

    body[i++] = p.device_id;
    body[i++] = p.version_id;
    body[i++] = p.flags;

    // TMP117_tem_x100 (int16, big-endian)
    body[i++] = (uint8_t)((p.TMP117_tem_x100 >> 8) & 0xFF);
    body[i++] = (uint8_t)(p.TMP117_tem_x100 & 0xFF);

    // CPU_tem_x100 (int16, big-endian)
    body[i++] = (uint8_t)((p.CPU_tem_x100 >> 8) & 0xFF);
    body[i++] = (uint8_t)(p.CPU_tem_x100 & 0xFF);

    // BAT_x100 (int8)
    body[i++] = (uint8_t)p.BAT_x100;
  }
}

struct DeviceConfig
{
  uint32_t device_id = 0;
  int32_t chip_temp_offset_x100 = 0; // tareado en centésimas
};

struct SensorData
{
  // TMP117
  int16_t temp_x100 = 0;
  bool i2c_ok = true;
  bool tmp_out_range = false;

  // VIN
  uint16_t adc_raw = 0;
  uint16_t vin_mV = 0;
  uint8_t vin_pct = 0;

  // MCU TEMP (chip)
  int16_t chipTemp_x100 = 0; // °C * 100
};

struct Flags
{
  bool i2c_fail = false;
  bool bat_low = false;
  bool tmp_fail = false;
};

static void initSerial(uint32_t baud = 115200)
{
  Serial.begin(baud);
  delay(200);

  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 1500))
  {
    delay(10);
  }
}

static BootMode detectBootMode(const button_mode::Config &bcfg)
{
  auto press = button_mode::detectAtBoot(bcfg, /*window*/ 1200, /*maxHold*/ 6000);
  return (press == button_mode::PressType::VeryLong) ? BootMode::config : BootMode::beacon;
}

static DeviceConfig loadDeviceConfigFromKV()
{
  DeviceConfig c;

  // ID entero (si no existe, default 0)
  c.device_id = kv_get_u32(KEY_DEVICE_ID, 0);

  // Offset de temperatura (si no existe, default 0)
  c.chip_temp_offset_x100 = kv_get_i32(KEY_CHIP_TEMP_OFFSET_X100, 0);

  return c;
}

// Lee temperatura del nRF52 en centésimas (x100) para evitar float
static int16_t readChipTemp_x100()
{
  NRF_TEMP->TASKS_START = 1;
  while (NRF_TEMP->EVENTS_DATARDY == 0)
  { /* wait */
  }
  NRF_TEMP->EVENTS_DATARDY = 0;

  int32_t raw_q = NRF_TEMP->TEMP; // 0.25°C por unidad

  NRF_TEMP->TASKS_STOP = 1;

  // raw_q * 0.25°C => (raw_q * 25) / 100 °C
  // En x100: raw_q * 25
  int32_t x100 = raw_q * 25;
  if (x100 > 32767)
    x100 = 32767;
  if (x100 < -32768)
    x100 = -32768;
  return (int16_t)x100;
}

static SensorData readSensors(int32_t chip_temp_offset_x100)
{
  SensorData d;

  // TMP117
  tmp117_begin();
  d.i2c_ok = tmp117_read_x100(d.temp_x100); // true = OK (según tu implementación)

  // VIN
  d.adc_raw = vin_adc_read_raw_avg();
  d.vin_mV = vin_adc_raw_to_vin_mV(d.adc_raw);
  d.vin_pct = vin_mV_to_percent(d.vin_mV);

  // MCU TEMP
  d.chipTemp_x100 = (int16_t)((int32_t)readChipTemp_x100() + chip_temp_offset_x100);

  d.tmp_out_range = d.temp_x100 > d.chipTemp_x100;

  return d;
}

static Flags computeFlags(const SensorData &d)
{
  Flags f;
  f.i2c_fail = !d.i2c_ok;
  f.bat_low = (d.vin_pct < 20);
  f.tmp_fail = d.tmp_out_range;
  return f;
}

static void logStatus(BootMode mode, const SensorData &d, const Flags &f, const DeviceConfig &dc)
{
  Serial.print("[BOOT MODE] ");
  Serial.println((int)mode);

  Serial.print("[KV] device_id = ");
  Serial.println(dc.device_id);

  Serial.print("[SENSOR] Temp TMP117 x100: ");
  Serial.println(d.temp_x100);

  Serial.print("[SENSOR] VIN mV: ");
  Serial.println(d.vin_mV);

  Serial.print("[SENSOR] BAT %: ");
  Serial.println(d.vin_pct);

  Serial.print("[SENSOR] MCU TEMP x100: ");
  Serial.println(d.chipTemp_x100);

  Serial.print("[FLAGS] I2C_FAIL: ");
  Serial.println(f.i2c_fail);

  Serial.print("[FLAGS] BAT_LOW: ");
  Serial.println(f.bat_low);

  Serial.print("[FLAGS] TMP FAIL: ");
  Serial.println(f.tmp_fail);

  Serial.print("[KV] chip_temp_offset_x100 = ");
  Serial.println(dc.chip_temp_offset_x100);

  uint64_t hwid =
      ((uint64_t)NRF_FICR->DEVICEID[1] << 32) |
      (uint64_t)NRF_FICR->DEVICEID[0];

  Serial.print("HWID: 0x");
  Serial.print((uint32_t)(hwid >> 32), HEX);
  Serial.print((uint32_t)hwid, HEX);
  Serial.println();
}

static uint8_t packFlagsByte(const Flags &f)
{
  uint8_t v = 0;
  if (f.i2c_fail)
    v |= 1 << 0;
  if (f.bat_low)
    v |= 1 << 1;
  if (f.tmp_fail)
    v |= 1 << 2;
  return v;
}

static BeaconPayload buildBeaconPayload(const DeviceConfig &dc,
                                        const SensorData &d,
                                        const Flags &f)
{
  BeaconPayload p{};
  p.company = 0xF510;

  // OJO: en tu struct device_id es uint8_t. Si dc.device_id es 32-bit, recorta:
  p.device_id = (uint8_t)(dc.device_id & 0xFF);

  p.version_id = 1;
  p.flags = packFlagsByte(f);

  p.TMP117_tem_x100 = d.temp_x100;
  p.CPU_tem_x100 = d.chipTemp_x100;

  // BAT_x100: si quieres guardar porcentaje (0..100) en int8_t
  p.BAT_x100 = (int8_t)d.vin_pct;

  return p;
}

static void goToSleep(deepsleep::Config &cfg)
{
  delay(300);
  adv_lib_end();
  // deepsleep::deepPowerOffMostThings();

  deepsleep::startLfclk();
  deepsleep::startWdt(cfg);
  deepsleep::disableUsbd();
  SysTick->CTRL = 0;
  deepsleep::sleepForeverWfe();
}

void setup()
{
  button_mode::Config bcfg;
  deepsleep::Config cfg;

  kv_begin();
  initSerial();

  DeviceConfig dc = loadDeviceConfigFromKV();

  deepsleep::blinkEntry(cfg);

  BootMode mode = detectBootMode(bcfg);

  if (mode == BootMode::beacon)
  {
    SensorData d = readSensors(dc.chip_temp_offset_x100);
    Flags f = computeFlags(d);

    BeaconPayload p = buildBeaconPayload(dc, d, f);
    uint8_t head[2];
    uint8_t body[8];

    beacon_to_head_body(p, head, sizeof(head), body, sizeof(body));

    adv_begin("FRIO-SENSOR");

    uint8_t cipher[32]; // sobra, pero está bien
    size_t cipher_len = 0;

    bool enc_ok = crypto_lib::encrypt_ecb_pkcs7(
        KEY,
        body, sizeof(body),
        cipher, sizeof(cipher),
        &cipher_len,
        16);

    if (!enc_ok || cipher_len != 16)
    {
      Serial.print("[CRYPTO] enc_ok=");
      Serial.print(enc_ok ? "true" : "false");
      Serial.print(" cipher_len=");
      Serial.println(cipher_len);
      goToSleep(cfg);
    }

    uint8_t pkt[2 + 16];
    memcpy(pkt + 0, head, 2);
    memcpy(pkt + 2, cipher, 16);

    bool ok = adv_publish_mfg(pkt, sizeof(pkt));

    Serial.print("[ADV] publish ok=");
    Serial.println(ok ? "true" : "false");

    logStatus(mode, d, f, dc);
    goToSleep(cfg);
  }
  else
  {
    Serial.println("[CONFIG] Mode");

    config_mode::Options opt;
    opt.idle_timeout_ms = 60 * 1000; // 60s sin uso => apagar
    opt.blink_period_ms = 250;       // parpadeo rápido

    config_mode::begin(opt);

    while (true)
    {
      if (config_mode::tick())
      {
        // acá llamas a tu sleep real
        goToSleep(cfg);
      }
      delay(10);
    }
  }
}

void loop() {}
