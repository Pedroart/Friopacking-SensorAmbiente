#include "netcfg.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Adafruit_TinyUSB.h> // opcional para Serial en nRF52
#include <EthernetENC.h>      // para usar IPAddress

using namespace Adafruit_LittleFS_Namespace;

// =============================================================
// Archivo de configuración y valores por defecto
// =============================================================
static const char* CFG_PATH = "/net.cfg";

static IPAddress ipDefault   (192, 168, 1, 50);
static IPAddress gwDefault   (192, 168, 1, 1);
static IPAddress maskDefault (255, 255, 255, 0);

// Estado actual en RAM
static IPAddress ipCur   = ipDefault;
static IPAddress gwCur   = gwDefault;
static IPAddress maskCur = maskDefault;

// =============================================================
// Helpers
// =============================================================
static bool parseLineKV(const String& line, String& key, String& val) {
  int eq = line.indexOf('=');
  if (eq <= 0) return false;
  key = line.substring(0, eq); key.trim();
  val = line.substring(eq + 1); val.trim();
  return key.length() > 0 && val.length() > 0;
}

bool parseIPv4(const String& s, IPAddress& out) {
  uint8_t parts[4] = {0,0,0,0};
  int last = 0, idx = 0;

  for (int i = 0; i < (int)s.length(); i++) {
    if (s[i] == '.' || i == (int)s.length() - 1) {
      int end = (s[i] == '.') ? i : i + 1;
      if (end <= last) return false;

      String token = s.substring(last, end);
      token.trim();
      if (token.length() == 0) return false;

      long v = token.toInt();
      if (v < 0 || v > 255) return false;
      if (idx > 3) return false;

      parts[idx++] = (uint8_t)v;
      last = i + 1;
    }
  }

  if (idx != 4) return false;
  out = IPAddress(parts[0], parts[1], parts[2], parts[3]);
  Serial.printf("[parseIPv4]: %d.%d.%d.%d\n", parts[0], parts[1], parts[2], parts[3]);
  return true;
}

static String ipToString(const IPAddress& ip) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  return String(buf);
}

// =============================================================
// Internos de FS
// =============================================================

// Cargar desde archivo
static bool loadFromFS() {
  File f(InternalFS);
  if (!f.open(CFG_PATH, FILE_O_READ)) {
    return false;
  }

  String content;
  char buf[64];
  while (true) {
    int n = f.read(buf, sizeof(buf) - 1);
    if (n <= 0) break;
    buf[n] = 0;
    content += buf;
  }
  f.close();

  IPAddress ipTmp = ipCur;
  IPAddress gwTmp = gwCur;
  IPAddress mkTmp = maskCur;

  int start = 0;
  while (start < content.length()) {
    int nl = content.indexOf('\n', start);
    if (nl < 0) nl = content.length();
    String line = content.substring(start, nl);
    start = nl + 1;

    line.trim();
    if (line.length() == 0 || line.startsWith("#")) continue;

    String k, v;
    if (!parseLineKV(line, k, v)) continue;

    if      (k.equalsIgnoreCase("ip"))   parseIPv4(v, ipTmp);
    else if (k.equalsIgnoreCase("gw"))   parseIPv4(v, gwTmp);
    else if (k.equalsIgnoreCase("mask")) parseIPv4(v, mkTmp);
  }

  ipCur   = ipTmp;
  gwCur   = gwTmp;
  maskCur = mkTmp;
  return true;
}

// Guardar a archivo
static bool saveToFS() {
  InternalFS.remove(CFG_PATH);

  File f(InternalFS);
  if (!f.open(CFG_PATH, FILE_O_WRITE)) {
    return false;
  }

  String s;
  s.reserve(96);
  s += "ip="   + ipToString(ipCur)   + "\n";
  s += "gw="   + ipToString(gwCur)   + "\n";
  s += "mask=" + ipToString(maskCur) + "\n";

  size_t wr = f.write(s.c_str(), s.length());
  f.close();
  return wr == s.length();
}

// =============================================================
// API Pública
// =============================================================

bool beginMemori() {
  InternalFS.begin();

  if (loadFromFS()) {
    Serial.println(F("[NetCfg] Archivo cargado correctamente."));
    return true;
  }

  Serial.println(F("[NetCfg] No existe archivo, creando con valores por defecto."));
  ipCur   = ipDefault;
  gwCur   = gwDefault;
  maskCur = maskDefault;
  return saveToFS();
}

bool setIP(const IPAddress& ip) {
  ipCur = ip;
  return saveToFS();
}

bool setGateway(const IPAddress& gw) {
  gwCur = gw;
  return saveToFS();
}

bool setMask(const IPAddress& mask) {
  maskCur = mask;
  return saveToFS();
}

bool setAll(const IPAddress& ip, const IPAddress& gw, const IPAddress& mask) {
  ipCur = ip;
  gwCur = gw;
  maskCur = mask;
  return saveToFS();
}

IPAddress getIP()   { return ipCur; }
IPAddress getGW()   { return gwCur; }
IPAddress getMask() { return maskCur; }

bool resetToDefaults() {
  ipCur   = ipDefault;
  gwCur   = gwDefault;
  maskCur = maskDefault;
  return saveToFS();
}
