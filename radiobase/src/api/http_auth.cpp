#include "http_auth.h"
#include <ArduinoJson.h>
#include "core/appState.h"

void registerAuthRoutes(AsyncWebServer &server)
{
    server.on("/api/login", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

            if (err)
            {
                JsonDocument res;
                res["ok"] = false;
                res["error"] = "invalid_json";

                char out[128];
                serializeJson(res, out);
                request->send(400, "application/json", out);
                return;
            }

            String username = doc["username"] | "";
            String password = doc["password"] | "";

            if (username.isEmpty() || password.isEmpty())
            {
                JsonDocument res;
                res["ok"] = false;
                res["error"] = "missing_fields";

                char out[128];
                serializeJson(res, out);
                request->send(400, "application/json", out);
                return;
            }

            UserEntry* user = Config.loadRole(users, username);

            if (user != nullptr && user->username == username && user->password_hash == password)
            {
                JsonDocument res;
                res["ok"] = true;
                res["user"] = user->username;
                res["role"] = static_cast<int>(user->role);

                char out[128];
                serializeJson(res, out);
                request->send(200, "application/json", out);
                return;
            }

            JsonDocument res;
            res["ok"] = false;
            res["error"] = "invalid_credentials";

            char out[128];
            serializeJson(res, out);
            request->send(401, "application/json", out);
        }
    );
}