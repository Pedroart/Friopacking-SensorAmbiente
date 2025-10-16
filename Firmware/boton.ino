// ---------- CONFIG HARDWARE ----------
static const uint8_t BUTTON_PIN  = D3;

// ---------- BOTÓN robusto ----------
static const uint32_t DEBOUNCE_MS   = 40;    // rebote
static const uint32_t BOOT_GUARD_MS = 1000;  // ignora durante arranque

enum class BtnSm { Idle, Pressed };

// Estado interno (debounced)
static BtnSm     btnSm            = BtnSm::Idle;
static bool      rawLevel         = true;   // lectura cruda
static bool      debouncedLevel   = true;   // HIGH = no pulsado (pullup)
static bool      lastRawLevel     = true;
static uint32_t  tLastRawEdge     = 0;     // último cambio crudo
static uint32_t  tPress           = 0;     // instante de pulsación (debounced)
static uint32_t  tBoot            = 0;     // instante de arranque

// Eventos latched (se limpian al leer)
static volatile bool evtPressed    = false;
static volatile bool evtReleased   = false;
static volatile uint32_t lastPressDurationMs = 0;

// ---------- API ----------
void buttonInit() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  rawLevel       = digitalRead(BUTTON_PIN);
  lastRawLevel   = rawLevel;
  debouncedLevel = rawLevel;
  btnSm          = rawLevel ? BtnSm::Idle : BtnSm::Pressed;
  tBoot          = millis();
  tLastRawEdge   = tBoot;
  evtPressed = evtReleased = false;
  lastPressDurationMs = 0;
}

// Llamar MUY seguido (loop)
void buttonTask() {
  uint32_t now = millis();

  // Protección de arranque
  if (now - tBoot < BOOT_GUARD_MS) {
    rawLevel       = digitalRead(BUTTON_PIN);
    lastRawLevel   = rawLevel;
    debouncedLevel = rawLevel;
    btnSm          = rawLevel ? BtnSm::Idle : BtnSm::Pressed;
    evtPressed = evtReleased = false;
    return;
  }

  // 1) Leer nivel crudo
  rawLevel = digitalRead(BUTTON_PIN); // HIGH=no pulsado, LOW=pulsado

  // 2) Detectar borde crudo y cronometrar para debounce
  if (rawLevel != lastRawLevel) {
    lastRawLevel = rawLevel;
    tLastRawEdge = now;               // comenzó el posible rebote
  }

  // 3) Si pasó el tiempo de debounce y el nivel estable cambió -> actualizar estado
  if ((now - tLastRawEdge) >= DEBOUNCE_MS && debouncedLevel != rawLevel) {
    debouncedLevel = rawLevel;

    // Transiciones de la SM sobre nivel DEBOUNCED
    if (debouncedLevel == LOW) {
      // Pulsación firme
      btnSm = BtnSm::Pressed;
      tPress = now;
      evtPressed = true;
    } else {
      // Liberación firme
      btnSm = BtnSm::Idle;
      evtReleased = true;
      lastPressDurationMs = now - tPress; // dura en ms
    }
  }
}

// --------- Helpers (consultas) ---------
bool buttonPressedEvent() {          // consume el evento "pressed"
  bool e = evtPressed;
  evtPressed = false;
  return e;
}

bool buttonReleasedEvent() {         // consume el evento "released"
  bool e = evtReleased;
  evtReleased = false;
  return e;
}

bool buttonIsDown() {                // estado actual, ya filtrado
  return (btnSm == BtnSm::Pressed);
}

uint32_t buttonHeldMs(uint32_t now = millis()) { // tiempo presionado si sigue abajo
  if (btnSm == BtnSm::Pressed) return now - tPress;
  return 0;
}

uint32_t buttonLastPressDuration() { // duración del último pulso completo
  return lastPressDurationMs;
}
