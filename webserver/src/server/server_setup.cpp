#include "server_setup.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

#include "api/api_wifi.h"
#include "api/api_static_files.h"
#include "api/api_not_found_handler.h"
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
  setupWebSocket(webserver::server);
  setupStaticFiles(webserver::server);
  webserver::server.onNotFound(&handleUrlNotFound);

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
