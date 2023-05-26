#include <Arduino.h>
#include <LittleFS.h>
#include "stm_comms/stm_comms.h"
#ifndef DISABLE_WIFI_SERVER
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
#endif

#ifndef DISABLE_BLE_SCALES
#include "scales/ble_scales.h"
#endif
#include "./log/log.h"

void initFS();

void setup() {
  LOG_INIT();
  stmCommsInit(Serial1);
  initFS();

#ifndef DISABLE_WIFI_SERVER
  wifiSetup();
  setupServer();
#ifdef REMOTE_LOGGING
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
#endif
#endif

#ifndef DISABLE_BLE_SCALES
  bleScalesInit();
#endif
}

void loop() {
  stmCommsReadData();
#ifndef DISABLE_BLE_SCALES
  bleScalesMaintainConnection();
#endif
#ifndef DISABLE_WIFI_SERVER
  wsCleanup();
#endif
}

// ------------------------------------------------------------------------
// ---------------- Handle STM communication messages ---------------------
// ------------------------------------------------------------------------
void onSensorStateSnapshotReceived(SensorStateSnapshot& sensorData) {
#ifndef DISABLE_WIFI_SERVER
  wsSendSensorStateSnapshotToClients(sensorData);
#endif
}

void onShotSnapshotReceived(ShotSnapshot& shotData) {
#ifndef DISABLE_WIFI_SERVER
  wsSendShotSnapshotToClients(shotData);
#endif
}
void onScalesTareReceived() {
#ifndef DISABLE_BLE_SCALES
  bleScalesTare();
#endif
}

// -----------------------------------------
// --------- Initialize SPIFFS -------------
// -----------------------------------------
void initFS() {
  if (!LittleFS.begin(true)) {
    LOG_INFO("An error has occurred while mounting LittleFS");
  }
  LOG_INFO("LittleFS mounted successfully");
}
