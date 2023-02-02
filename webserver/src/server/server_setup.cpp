#include "server_setup.h"

#include "websocket.h"
#include "wifi_api.h"

AsyncWebServer server(80);

void setupStaticFiles(AsyncWebServer& server) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) { request->send(LittleFS, "/index.html", "text/html"); });
    server.serveStatic("/", LittleFS, "/");
}

void setupServer() {
    setupWifiApi(server);
    setupStaticFiles(server);
    setupWebSocket(server);

    server.begin();
}
