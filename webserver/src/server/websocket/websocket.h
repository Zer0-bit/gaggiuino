#ifndef WEBSOCKET_API_H
#define WEBSOCKET_API_H

#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "string"
#include "sensors_state.h"
#include "system_state.h"

void setupWebSocket(AsyncWebServer& server);
void wsCleanup();
void wsSendSensorStateSnapshotToClients(const SensorStateSnapshot& snapshot);
void wsSendShotSnapshotToClients(const ShotSnapshot& snapshot);
void wsSendLog(std::string log, std::string source = "webserver");
void wsSendSystemStateToClients(const SystemState& systemState);

#endif
