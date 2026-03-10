#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>

WebServer server(80);

static const char* contentTypeFor(const String& path) {
  if (path.endsWith(".html")) return "text/html; charset=utf-8";
  if (path.endsWith(".js"))   return "application/javascript";
  if (path.endsWith(".css"))  return "text/css";
  if (path.endsWith(".svg"))  return "image/svg+xml";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".ico"))  return "image/x-icon";
  return "application/octet-stream";
}

static bool hasExtension(const String& p) {
  int lastSlash = p.lastIndexOf('/');
  int lastDot = p.lastIndexOf('.');
  return lastDot > lastSlash; // hay "." después del último "/"
}

static void servePath(String path) {
  if (path == "/") path = "/index.html";

  // 1) SPA fallback: si NO es archivo real (sin extensión) y no es /assets o /api,
  // entonces sirve index.html para que el router del frontend resuelva la ruta.
  if (!hasExtension(path) && !path.startsWith("/assets/") && !path.startsWith("/api/")) {
    path = "/index.html";
  }

  bool isAsset = path.startsWith("/assets/");
  const char* cacheHdr = isAsset ? "public, max-age=31536000, immutable" : "no-cache";

  String gz = path + ".gz";
  bool hasGz = LittleFS.exists(gz);
  String filePath = hasGz ? gz : path;

  if (!LittleFS.exists(filePath)) {
    server.send(404, "text/plain", "Not found");
    return;
  }

  File f = LittleFS.open(filePath, "r");
  if (!f) {
    server.send(500, "text/plain", "Open fail");
    return;
  }

  if (hasGz) {
    //server.sendHeader("Content-Encoding", "gzip");   // <-- CLAVE
    server.sendHeader("Vary", "Accept-Encoding");
  }
  server.sendHeader("Cache-Control", cacheHdr);

  server.streamFile(f, contentTypeFor(path)); // usa el tipo del archivo original (sin .gz)
  f.close();
}

void setup() {
  Serial.begin(115200);

  // AP rápido para probar
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-TEST", "12345678");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
  }

  server.on("/", HTTP_GET, [](){ servePath("/"); });
  server.onNotFound([](){ servePath(server.uri()); });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}