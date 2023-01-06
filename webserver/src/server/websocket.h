#ifndef WEBSOCKET_API_H
#define WEBSOCKET_API_H

#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include "mcu_comms.h"

void setupWebSocket(AsyncWebServer& server);
void wsCleanup();
void wsSendSensorStateSnapshotToClients(SensorStateSnapshot& snapshot);
void wsSendShotSnapshotToClients(ShotSnapshot& snapshot);

#endif
