#include "adv_lib.h"
#include <bluefruit.h>

/*
  Legacy Advertising (ADV) máximo: 31 bytes.
  Tu función SIEMPRE incluye:
   - Flags: 3 bytes  (02 01 XX)
   - TxPower: 3 bytes (02 0A XX)
  Manufacturer Specific Data (MSD) ocupa:
   - Overhead 4 bytes (len + type + companyID(2)) + payload_len

  Total estimado:
    used = 3 (flags) + 3 (txpower) + 4 (msd overhead) + payload_len
         = payload_len + 10

  Por tanto:
    payload_len <= 21 para que used <= 31
*/

static uint16_t g_min_units = ADV_INTERVAL_MIN_UNITS;
static uint16_t g_max_units = ADV_INTERVAL_MAX_UNITS;

static inline size_t flags_bytes() { return 3; }
static inline size_t txpower_bytes() { return 3; }
static inline size_t msd_overhead() { return 4; } // len+type+companyId(2)

size_t adv_overhead_bytes()
{
  size_t used = 0;

#if ADV_ALWAYS_FLAGS
  used += flags_bytes();
#endif

#if ADV_ALWAYS_TXPOWER
  used += txpower_bytes();
#endif

  used += msd_overhead();
  return used;
}

int adv_max_mfg_payload()
{
  const int maxTotal = ADV_LEGACY_MAX_BYTES;
  int maxPayload = (int)maxTotal - (int)adv_overhead_bytes();
  return maxPayload; // típico: 31 - 10 = 21
}

bool adv_will_fit_mfg(size_t payload_len)
{
  return ((int)payload_len <= adv_max_mfg_payload());
}

void adv_begin(const char *name)
{
  Bluefruit.begin();
  Bluefruit.setName(name);
  Bluefruit.autoConnLed(false);

  // Intervalo fijo
  Bluefruit.Advertising.setInterval(g_min_units, g_max_units);
}

void adv_set_name(const char *name)
{
  Bluefruit.setName(name);
}

void adv_set_interval_units(uint16_t min_units, uint16_t max_units)
{
  g_min_units = min_units;
  g_max_units = max_units;
  Bluefruit.Advertising.setInterval(g_min_units, g_max_units);
}

void adv_clear()
{
  Bluefruit.Advertising.clearData();
  Bluefruit.ScanResponse.clearData(); // aunque no usemos SR
}

void adv_stop()
{
  Bluefruit.Advertising.stop();
}

void adv_start(uint16_t seconds)
{
  Bluefruit.Advertising.start(seconds);
}

bool adv_publish_mfg(const uint8_t *data, size_t len, uint16_t seconds)
{
  // 1) Verifica tamaño (sin SR, con flags+txpower fijos)
  if (!adv_will_fit_mfg(len))
  {
    return false;
  }

  // 2) Parar antes de modificar
  adv_stop();

  // 3) Limpiar
  adv_clear();

  // 4) Agregar flags (siempre en tu diseño)
#if ADV_ALWAYS_FLAGS
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
#endif

  // 5) Agregar TxPower (SIEMPRE)
#if ADV_ALWAYS_TXPOWER
  Bluefruit.Advertising.addTxPower();
#endif

  // 6) Agregar manufacturer data (payload)
  Bluefruit.Advertising.addManufacturerData(data, (uint16_t)len);

  // 7) Arrancar
  adv_start(seconds);
  return true;
}

void adv_lib_end()
{
  Bluefruit.Advertising.stop();
  Bluefruit.disconnect(0); // si aplica
  sd_softdevice_disable(); // o Bluefruit.end() si lo tienes disponible
}