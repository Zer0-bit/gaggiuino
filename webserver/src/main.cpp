#include <Arduino.h>
#include "task_config.h"
#include "filesystem/filesystem.h"
#include "stm_comms/stm_comms.h"
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
#include "scales/ble_scales.h"
#include "persistence/persistence.h"
#include "./log/log.h"

GaggiaSettings settings;

void setup() {
  LOG_INIT();
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
  initFS();
  persistence::init();
  stmCommsInit(Serial1);
  wifiSetup();
  webServerSetup();
  bleScalesInit();
  vTaskDelete(NULL);     //Delete own task by passing NULL
}

void loop() {
  vTaskDelete(NULL);     //Delete own task by passing NULL
}

// ------------------------------------------------------------------------
// ---------------- Handle STM communication messages ---------------------
// ------------------------------------------------------------------------
void onSensorStateSnapshotReceived(const SensorStateSnapshot& sensorData) {
  wsSendSensorStateSnapshotToClients(sensorData);
}

void onShotSnapshotReceived(const ShotSnapshot& shotData) {
  wsSendShotSnapshotToClients(shotData);
}

void onScalesTareReceived() {
  bleScalesTare();
}

void onGaggiaSettingsRequested() {
  stmCommsSendGaggiaSettings(settings);
}

void onProfileRequested() {
  stmCommsSendProfile(persistence::getActiveProfile());
}
