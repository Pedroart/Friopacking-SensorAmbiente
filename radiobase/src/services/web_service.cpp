#include "web_service.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "api/http_routes.h"
#include "api/ws_routes.h"
#include "api/http_auth.h"


static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

bool WebService::begin()
{
    registerHttpRoutes(server);
    registerAuthRoutes(server);
    registerWsRoutes(ws);

    server.addHandler(&ws);
    server.begin();
    return true;
}

void WebService::loop()
{
    ws.cleanupClients();
}

WebService webService;
