#include "server_setup.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "LittleFS.h"

#include "api/api_wifi.h"
#include "api/api_static_files.h"
#include "api/api_not_found_handler.h"
#include "api/api_settings.h"
#include "api/api_profile.h"
#include "api/api_system_state.h"
#include "websocket/websocket.h"
#include "../task_config.h"
#include "../log/log.h"

namespace webserver {
  const int PORT_NUMBER = 80;
  AsyncWebServer server(PORT_NUMBER);
}

void webServerTask(void* params);

void webServerSetup() {
  setupWifiApi(webserver::server);
  setupSettingsApi(webserver::server);
  setupProfileApi(webserver::server);
  setupSystemStateApi(webserver::server);
  setupWebSocket(webserver::server);
  setupStaticFiles(webserver::server);
  webserver::server.onNotFound(&handleUrlNotFound);

  // Ggzipped files still not being handled. ::shrug::
  webserver::server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS , "/index.html.gz"); //request->beginResponse( LittleFS, "/index.html.gz");
    if (request->url().endsWith(".gz")) {
      response->addHeader("Content-Encoding", "gzip");
    } else {
      response = request->beginResponse(LittleFS, "/index.html");
    }

    request->send(response);
  });

  // webserver::server.serveStatic("/", LittleFS, "/");

  LOG_INFO("Starting up web server on port %d...", webserver::PORT_NUMBER);
  webserver::server.begin();

  xTaskCreateUniversal(webServerTask, "webserverMaintenance", configMINIMAL_STACK_SIZE + 100, NULL, PRIORITY_WEBSERVER_MAINTENANCE, NULL, CORE_WEBSERVER_MAINTENANCE);
}

void webServerTask(void* params) {
  while(true) {
    wsCleanup();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
