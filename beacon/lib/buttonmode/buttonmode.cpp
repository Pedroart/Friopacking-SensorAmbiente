#include "buttonmode.h"
#include <Adafruit_TinyUSB.h> 

namespace button_mode
{

    static bool readRaw(const Config &cfg)
    {
        int v = digitalRead(cfg.pin);
        if (cfg.activeLow)
            return (v == LOW);
        return (v == HIGH);
    }

    void begin(const Config &cfg)
    {
        if (cfg.usePullup)
        {
            pinMode(cfg.pin, cfg.activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
        }
        else
        {
            pinMode(cfg.pin, INPUT);
        }
    }

    bool isPressedNow(const Config &cfg)
    {
        return readRaw(cfg);
    }

    static bool waitStablePressed(const Config &cfg, uint32_t timeoutMs)
    {
        uint32_t t0 = millis();
        while (millis() - t0 < timeoutMs)
        {
            if (readRaw(cfg))
            {
                // debounce: confirmar que sigue presionado después de debounceMs
                delay(cfg.debounceMs);
                if (readRaw(cfg))
                    return true;
            }
            delay(1);
        }
        return false;
    }

    PressType detectAtBoot(const Config &cfg, uint32_t windowMs, uint32_t maxHoldMs)
    {
        begin(cfg);

        // Ventana para que el usuario presione al arranque
        if (!waitStablePressed(cfg, windowMs))
            return PressType::None;
        Serial.print("[buttonmode] Boton Presionado\ns");
        // Ya está presionado: medir duración hasta que suelte o maxHoldMs
        uint32_t tPress = millis();
        while (readRaw(cfg) && (millis() - tPress < maxHoldMs))
        {
            delay(2);
        }
        uint32_t held = millis() - tPress;

        if (held >= cfg.veryLongMinMs)
            return PressType::VeryLong;
        if (held >= cfg.longMinMs)
            return PressType::Long;
        if (held <= cfg.shortMaxMs)
            return PressType::Short;

        // Zona intermedia: trátalo como Long suave
        return PressType::Long;
    }

} // namespace button_mode
