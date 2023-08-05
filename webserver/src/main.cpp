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
  blescales::init();
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
void onSystemStateReceived(const SystemState& systemState) {
  state::updateSystemState(systemState);
}
void onScalesTareReceived() {
  LOG_INFO("STM sent tare command");
  blescales::tare();
}
void onGaggiaSettingsRequested() {
  LOG_INFO("STM request active settings");
  stmCommsSendGaggiaSettings(state::getSettings());
}
void onProfileRequested() {
  LOG_INFO("STM request active profile");
  stmCommsSendProfile(state::getActiveProfile());
}
void onNotification(const Notification& notification) {
  wsSendNotification(notification);
}
void onDescalingProgressReceived(const DescalingProgress& progress) {
  wsSendDescalingProgress(progress);
}

// ------------------------------------------------------------------------
// ------------------ Handle state updated callbacks ----------------------
// ------------------------------------------------------------------------
void state::onActiveProfileUpdated(const Profile& profile) {
  stmCommsSendProfile(state::getActiveProfile());
  wsSendActiveProfileUpdated();
}
void state::onAllSettingsUpdated(const GaggiaSettings& settings) {
  stmCommsSendGaggiaSettings(state::getSettings());
  wsSendSettingsUpdated();
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
void state::onScalesSettingsUpdated(const ScalesSettings& settings) {
  stmCommsSendScalesSettings(settings);
}
void state::onSystemStateUpdated(const SystemState& systemState) {
  wsSendSystemStateToClients(systemState);
}
void state::onUpdateSystemStateCommandSubmitted(const UpdateSystemStateComand& command) {
  stmCommsSendUpdateSystemState(command);
}
void state::onConnectedBleScalesUpdated(const blescales::Scales& scales) {
  if (scales.address.length() == 0) {
    stmCommsSendScaleDisconnected();
  }
  wsSendConnectedBleScalesUpdated(scales);
}

// ------------------------------------------------------------------------
// -------------------- Handle ble scales callbacks -----------------------
// ------------------------------------------------------------------------
void blescales::onWeightReceived(float weight) {
  stmCommsSendWeight(weight);
}
