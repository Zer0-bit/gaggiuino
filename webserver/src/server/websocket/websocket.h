#ifndef WEBSOCKET_API_H
#define WEBSOCKET_API_H

#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "mcu_comms.h"

void setupWebSocket(AsyncWebServer& server);
void wsCleanup();
void wsSendSensorStateSnapshotToClients(const SensorStateSnapshot& snapshot);
void wsSendShotSnapshotToClients(const ShotSnapshot& snapshot);
void wsSendLog(std::string log, std::string source = "webserver");

#endif
