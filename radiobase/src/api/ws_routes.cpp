#include "ws_routes.h"

static AsyncWebSocket *g_ws = nullptr;

static void onWsEvent(
    AsyncWebSocket *server,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WS cliente conectado: %u\n", client->id());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WS cliente desconectado: %u\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        Serial.printf("WS data de cliente: %u\n", client->id());
    }
}

void registerWsRoutes(AsyncWebSocket &ws)
{
    g_ws = &ws;
    ws.onEvent(onWsEvent);
}

void wsBroadcastText(const char *msg)
{
    if (g_ws != nullptr)
    {
        g_ws->textAll(msg);
    }
}