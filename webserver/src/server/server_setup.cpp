#include "server_setup.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

#include "api/api_wifi.h"
#include "api/api_static_files.h"
#include "api/api_not_found_handler.h"
#include "websocket/websocket.h"
#include "../log/log.h"

namespace webserver {
  const int PORT_NUMBER = 80;
  AsyncWebServer server(PORT_NUMBER);
}

void webServerSetup() {
  setupWifiApi(webserver::server);
  setupWebSocket(webserver::server);
  setupStaticFiles(webserver::server);
  webserver::server.onNotFound(&handleUrlNotFound);

  LOG_INFO("Starting up web server on port %d...", webserver::PORT_NUMBER);
  webserver::server.begin();
}

void webServerTask() {

}
