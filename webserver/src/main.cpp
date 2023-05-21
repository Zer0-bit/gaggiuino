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

void initFS();

void setup() {
  Serial.begin(460800);
  stmCommsInit(Serial1);
  initFS();

#ifndef DISABLE_WIFI_SERVER
  wifiSetup();
  setupServer();
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
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}
