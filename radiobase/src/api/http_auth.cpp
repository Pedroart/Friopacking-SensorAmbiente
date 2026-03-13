#include "http_auth.h"
#include <ArduinoJson.h>
#include "core/appState.h"

void registerAuthRoutes(AsyncWebServer &server)
{
    // LOGIN
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

            UserEntry *user = Config.loadRole(users, username);

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

    // CAMBIO DE CONTRASEÑA
    server.on("/api/change-password", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

            if (err)
            {
                JsonDocument res;
                res["success"] = false;
                res["message"] = "JSON inválido";

                char out[128];
                serializeJson(res, out);
                request->send(400, "application/json", out);
                return;
            }

            String usuario     = doc["usuario"] | "";
            String oldPassword = doc["oldPassword"] | "";
            String newPassword = doc["newPassword"] | "";

            if (usuario.isEmpty() || oldPassword.isEmpty() || newPassword.isEmpty())
            {
                JsonDocument res;
                res["success"] = false;
                res["message"] = "missing_fields";

                char out[128];
                serializeJson(res, out);
                request->send(400, "application/json", out);
                return;
            }

            UserEntry *user = Config.loadRole(users, usuario);

            if (user == nullptr)
            {
                JsonDocument res;
                res["success"] = false;
                res["message"] = "user_not_found";

                char out[128];
                serializeJson(res, out);
                request->send(400, "application/json", out);
                return;
            }

            if (user->password_hash != oldPassword)
            {
                JsonDocument res;
                res["success"] = false;
                res["message"] = "incorrect_current_password";

                char out[128];
                serializeJson(res, out);
                request->send(403, "application/json", out);
                return;
            }

            // Actualizar contraseña
            user->password_hash = newPassword;

            // Guardar cambios en memoria persistente
            Config.saveUsers(users);   // <-- ajusta esto a tu función real

            JsonDocument res;
            res["success"] = true;
            res["message"] = "updated_password";

            char out[128];
            serializeJson(res, out);
            request->send(200, "application/json", out);
        }
    );
}