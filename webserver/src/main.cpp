#include <Arduino.h>
#include <LittleFS.h>
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
#include "stm_comms/stm_comms.h"

void initFS();

void setup() {
  Serial.begin(460800);
  stmCommsInit(Serial1);
  initFS();
  wifiSetup();
  setupServer();
}

void loop() {
  stmCommsReadData();
  wsCleanup();
}

// Handle STM data
void onSensorStateSnapshotReceived(SensorStateSnapshot& sensorData) {
  wsSendSensorStateSnapshotToClients(sensorData);
}

void onShotSnapshotReceived(ShotSnapshot& shotData) {
  wsSendShotSnapshotToClients(shotData);
}

// Initialize SPIFFS
void initFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}
