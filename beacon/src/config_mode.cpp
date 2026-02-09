#include "config_mode.h"

#include <bluefruit.h>
#include "kvstore.h"   // tienes kv_begin(), kv_get_u32(), kv_set_u32()
#include "deepsleep.h" // para apagar (tu main ya tiene goToSleep, pero acá solo avisamos)
#include "config.h"    // para las claves de KV y otras constantes
namespace config_mode {

// ---- KV keys (usa las que quieras) ----
static const char* KEY_DEVICE_ID_U8 = KEY_DEVICE_ID;
static const char* KEY_TX_POWER_U8  = KEY_TX_POWER;

// ---- UUIDs (ejemplo 128-bit) ----
static const uint8_t UUID_SVC_CFG[16] = {
  0x8f,0x32,0x1a,0x20,0x77,0x45,0x4a,0x9e,0x9a,0x2c,0x11,0x90,0x00,0x00,0x10,0x01
};

static const uint8_t UUID_CHR_DEVICE_ID[16] = {
  0x8f,0x32,0x1a,0x20,0x77,0x45,0x4a,0x9e,0x9a,0x2c,0x11,0x90,0x00,0x01,0x10,0x01
};

static const uint8_t UUID_CHR_TX_POWER[16] = {
  0x8f,0x32,0x1a,0x20,0x77,0x45,0x4a,0x9e,0x9a,0x2c,0x11,0x90,0x00,0x02,0x10,0x01
};

// ---- BLE objects ----
static BLEService cfgSvc(UUID_SVC_CFG);
static BLECharacteristic chrDeviceId(UUID_CHR_DEVICE_ID);
static BLECharacteristic chrTxPower(UUID_CHR_TX_POWER);

// ---- state ----
static Options g_opt;
static uint32_t g_last_activity_ms = 0;

static uint32_t g_last_blink_ms = 0;
static bool g_led_on = false;

static inline uint8_t clamp0_50(uint8_t v) { return (v > 50) ? 50 : v; }

static void touch()
{
  g_last_activity_ms = millis();
}

uint32_t lastActivityMs()
{
  return g_last_activity_ms;
}

// Guarda como u32 en KV (para no depender de kv_get_u8/kv_set_u8)
static uint8_t kv_get_u8_compat(const char* key, uint8_t defv)
{
  uint32_t v = kv_get_u32(key, defv);
  return (uint8_t)(v & 0xFF);
}

static void kv_set_u8_compat(const char* key, uint8_t v)
{
  kv_set_u32(key, (uint32_t)v);
}

// ---- callbacks ----
static void onConnect(uint16_t)
{
  touch();
}

static void onDisconnect(uint16_t, uint8_t)
{
  touch();
}

static void onWriteDeviceId(uint16_t, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  if (len != 1) return;
  uint8_t v = clamp0_50(data[0]);
  kv_set_u8_compat(KEY_DEVICE_ID_U8, v);
  chr->write(&v, 1);
  touch();
}

static void onWriteTxPower(uint16_t, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  if (len != 1) return;
  uint8_t v = clamp0_50(data[0]);
  kv_set_u8_compat(KEY_TX_POWER_U8, v);
  chr->write(&v, 1);
  touch();
}

Values load()
{
  Values out;
  out.device_id = clamp0_50(kv_get_u8_compat(KEY_DEVICE_ID_U8, 0));
  out.tx_power  = clamp0_50(kv_get_u8_compat(KEY_TX_POWER_U8, 0));
  return out;
}

static void led_init()
{
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif
}

static void led_set(bool on)
{
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
#else
  (void)on;
#endif
}

static void start_adv()
{
  Bluefruit.Advertising.stop();
  Bluefruit.ScanResponse.clearData();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(cfgSvc);
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 244); // ~100..152ms
  Bluefruit.Advertising.start(0);
}

void begin(const Options& opt)
{
  g_opt = opt;

  led_init();

  // BLE stack
  Bluefruit.begin();
  Bluefruit.setName("FRIO-CONFIG");
  Bluefruit.autoConnLed(false); // tú controlas el parpadeo
  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);

  // Servicio + chars
  cfgSvc.begin();

  // device_id (1 byte)
  chrDeviceId.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrDeviceId.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrDeviceId.setFixedLen(1);
  chrDeviceId.setWriteCallback(onWriteDeviceId);
  chrDeviceId.begin();

  // tx_power (1 byte)
  chrTxPower.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  chrTxPower.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrTxPower.setFixedLen(1);
  chrTxPower.setWriteCallback(onWriteTxPower);
  chrTxPower.begin();

  // cargar valores iniciales y exponerlos en read
  Values v = load();
  chrDeviceId.write(&v.device_id, 1);
  chrTxPower.write(&v.tx_power, 1);

  start_adv();
  touch(); // arranca el “reloj” de actividad
}

static void tick_blink()
{
  const uint32_t now = millis();
  if ((uint32_t)(now - g_last_blink_ms) >= g_opt.blink_period_ms) {
    g_last_blink_ms = now;
    g_led_on = !g_led_on;
    led_set(g_led_on);
  }
}

bool tick()
{
  tick_blink();

  const uint32_t now = millis();
  const uint32_t idle = (uint32_t)(now - g_last_activity_ms);

  // “Actividad” mínima: si está conectado, consideramos activo
  if (Bluefruit.connected()) {
    touch();
  }

  // Auto-apagado si excede timeout
  if (idle >= g_opt.idle_timeout_ms) {
    led_set(false);
    return true; // señal: debe apagarse
  }

  return false;
}

} // namespace config_mode
