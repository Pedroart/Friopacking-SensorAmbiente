#include "web_service.h"
#include <AsyncWebServer_ESP32_SC_W5500.h>
#include <LittleFS.h>

static const char *contentTypeFor(const String &path)
{
    if (path.endsWith(".html"))
        return "text/html; charset=utf-8";
    if (path.endsWith(".js"))
        return "application/javascript";
    if (path.endsWith(".css"))
        return "text/css";
    if (path.endsWith(".svg"))
        return "image/svg+xml";
    if (path.endsWith(".json"))
        return "application/json";
    if (path.endsWith(".ico"))
        return "image/x-icon";
    if (path.endsWith(".png"))
        return "image/png";
    if (path.endsWith(".jpg"))
        return "image/jpeg";
    if (path.endsWith(".woff2"))
        return "font/woff2";
    return "application/octet-stream";
}

static bool hasExtension(const String &p)
{
    int lastSlash = p.lastIndexOf('/');
    int lastDot = p.lastIndexOf('.');
    return lastDot > lastSlash;
}

static void servePath(AsyncWebServerRequest *request)
{
    String path = request->url();

    if (path == "/")
        path = "/index.html";

    // SPA fallback:
    // si no parece archivo y no es /api ni /ws, servir index.html
    if (!hasExtension(path) &&
        !path.startsWith("/api/") &&
        !path.startsWith("/ws"))
    {
        path = "/index.html";
    }

    bool isAsset = path.startsWith("/assets/");
    const char *cacheHdr = isAsset
                               ? "public, max-age=31536000, immutable"
                               : "no-cache";

    String gzPath = path + ".gz";
    bool hasGz = LittleFS.exists(gzPath);
    String finalPath = hasGz ? gzPath : path;

    if (!LittleFS.exists(finalPath))
    {
        request->send(404, "text/plain", "Not found");
        return;
    }

    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, finalPath, contentTypeFor(path));

    if (hasGz)
    {
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Vary", "Accept-Encoding");
    }

    response->addHeader("Cache-Control", cacheHdr);
    request->send(response);
}

void registerHttpPaths(AsyncWebServer &server)
{
    // Ruta raíz
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { servePath(request); });

    // Todo lo no encontrado pasa por el frontend
    server.onNotFound([](AsyncWebServerRequest *request)
                      { servePath(request); });
}