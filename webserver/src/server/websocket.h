#ifndef WEBSOCKET_API_H
#define WEBSOCKET_API_H

#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

void setupWebSocket(AsyncWebServer& server);
void wsCleanup();
void wsSendSensorStatesToClients(
  long timeInShot,
  float temp,
  float pressure,
  float flow,
  float weight
);

#endif
