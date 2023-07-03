#include <Arduino.h>
#include "task_config.h"
#include "filesystem/filesystem.h"
#include "stm_comms/stm_comms.h"
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
#include "scales/ble_scales.h"
#include "persistence/persistence.h"
#include "state/state.h"
#include "state/state.h"
#include "./log/log.h"

void setup() {
  LOG_INIT();
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
  initFS();
  persistence::init();
  state::init();
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
// ----------------------- Handle STM callbacks ---------------------------
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
  stmCommsSendGaggiaSettings(state::getSettings());
}
void onProfileRequested() {
  stmCommsSendProfile(state::getActiveProfile());
}
void onSystemStateReceived(const SystemState& systemState) {
  wsSendSystemStateToClients(systemState);
}

// ------------------------------------------------------------------------
// ------------------ Handle state updated callbacks ----------------------
// ------------------------------------------------------------------------
void state::onActiveProfileUpdated(const Profile& profile) {
  stmCommsSendProfile(state::getActiveProfile());
}
void state::onAllSettingsUpdated(const GaggiaSettings& settings) {
  stmCommsSendGaggiaSettings(state::getSettings());
}
void state::onBrewSettingsUpdated(const BrewSettings& settings) {
  stmCommsSendBrewSettings(settings);
}
void state::onBoilerSettingsUpdated(const BoilerSettings& settings) {
  stmCommsSendBoilerSettings(settings);
}
void state::onLedSettingsUpdated(const LedSettings& settings) {
  stmCommsSendLedSettings(settings);
}
void state::onSystemSettingsUpdated(const SystemSettings& settings) {
  stmCommsSendSystemSettings(settings);
}
void state::onOperationModeUpdated(const OperationMode operationMode) {
  stmCommsSendUpdateOperationMode(operationMode);
}
