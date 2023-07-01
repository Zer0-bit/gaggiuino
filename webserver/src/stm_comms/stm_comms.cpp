#include "stm_comms.h"
#include "../task_config.h"
#include "vector"
#include "proto/proto_serializer.h"
#include "proto/message_converters.h"
#include "proto/profile_converters.h"
#include "proto/settings_converters.h"

namespace {
  McuComms mcuComms;
  SemaphoreHandle_t mcucLock = xSemaphoreCreateRecursiveMutex();
}

void stmCommsTask(void* params);
void onMessageReceived(McuCommsMessageType type, std::vector<uint8_t>& data);
void stmCommsInit(HardwareSerial& serial) {
  serial.setRxBufferSize(256);
  serial.setTxBufferSize(256);
  serial.begin(460800);

  // mcuComms.setDebugPort(&Serial);
  mcuComms.begin(serial);

  // Set callbacks
  mcuComms.setMessageReceivedCallback(onMessageReceived);

  xTaskCreateUniversal(stmCommsTask, "stmComms", configMINIMAL_STACK_SIZE + 2400, NULL, PRIORITY_STM_COMMS, NULL, CORE_STM_COMMS);
}

void stmCommsTask(void* params) {
  for (;;) {
    stmCommsReadData();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void onDataRequest(McuCommsMessageType requestedDataType) {
  switch (requestedDataType) {
  case McuCommsMessageType::MCUC_DATA_ALL_SETTINGS: {
    onGaggiaSettingsRequested();
    break;
  }
  case McuCommsMessageType::MCUC_DATA_PROFILE: {
    onProfileRequested();
    break;
  }
  default: // Ignore unhandled request
    break;
  }
}

void onMessageReceived(McuCommsMessageType messageType, std::vector<uint8_t>& data) {
  switch (messageType) {
  case McuCommsMessageType::MCUC_DATA_SENSOR_STATE_SNAPSHOT: {
    SensorStateSnapshot snapshot;
    ProtoSerializer::deserialize<SensorStateSnapshotConverter>(data, snapshot);
    onSensorStateSnapshotReceived(snapshot);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_SHOT_SNAPSHOT: {
    ShotSnapshot snapshot;
    ProtoSerializer::deserialize<ShotSnapshotConverter>(data, snapshot);
    onShotSnapshotReceived(snapshot);
    break;
  }
  case McuCommsMessageType::MCUC_CMD_REMOTE_SCALES_TARE: {
    onScalesTareReceived();
    break;
  }
  case McuCommsMessageType::MCUC_REQ_DATA: {
    McuCommsRequestData dataRequest;
    ProtoSerializer::deserialize<McuCommsRequestDataConverter>(data, dataRequest);
    onDataRequest(dataRequest.type);
  }
  default:
    break;
  }
}

void stmCommsReadData() {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.readDataAndTick();
  xSemaphoreGiveRecursive(mcucLock);
}

void stmCommsSendWeight(float weight) {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_REMOTE_SCALES_WEIGHT,
    ProtoSerializer::serialize<MeasurementConverter>(Measurement{ .value = weight })
  );
  xSemaphoreGiveRecursive(mcucLock);
}

void stmCommsSendScaleDisconnected() {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.sendMessage(McuCommsMessageType::MCUC_DATA_REMOTE_SCALES_DISCONNECTED);
  xSemaphoreGiveRecursive(mcucLock);
}

void stmCommsSendGaggiaSettings(const GaggiaSettings& settings) {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_ALL_SETTINGS,
    ProtoSerializer::serialize<GaggiaSettingsConverter>(settings)
  );
  xSemaphoreGiveRecursive(mcucLock);
}
void stmCommsSendProfile(const Profile& profile) {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_PROFILE,
    ProtoSerializer::serialize<ProfileConverter>(profile)
  );
  xSemaphoreGiveRecursive(mcucLock);
}
