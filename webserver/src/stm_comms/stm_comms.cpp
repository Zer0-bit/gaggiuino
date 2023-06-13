#include "stm_comms.h"
#include "../task_config.h"

namespace {
  McuComms mcuComms;
  SemaphoreHandle_t mcucLock = xSemaphoreCreateRecursiveMutex();
}

void stmCommsTask(void* params);
void stmCommsInit(HardwareSerial& serial) {
  serial.setRxBufferSize(256);
  serial.setTxBufferSize(256);
  serial.begin(460800);

  // mcuComms.setDebugPort(&Serial);
  mcuComms.begin(serial);

  // Set callbacks
  mcuComms.setShotSnapshotCallback(onShotSnapshotReceived);
  mcuComms.setSensorStateSnapshotCallback(onSensorStateSnapshotReceived);
  mcuComms.setRemoteScalesTareCommandCallback(onScalesTareReceived);

  xTaskCreateUniversal(stmCommsTask, "stmComms", configMINIMAL_STACK_SIZE + 2400, NULL, PRIORITY_STM_COMMS, NULL, CORE_STM_COMMS);
}

void stmCommsTask(void* params) {
  for (;;) {
    stmCommsReadData();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void stmCommsReadData() {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.readDataAndTick();
  xSemaphoreGiveRecursive(mcucLock);
}

void stmCommsSendWeight(float weight) {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.sendRemoteScalesWeight(weight);
  xSemaphoreGiveRecursive(mcucLock);
}

void stmCommsSendScaleDisconnected() {
  if (xSemaphoreTakeRecursive(mcucLock, portMAX_DELAY) == pdFALSE) return;
  mcuComms.sendRemoteScalesDisconnected();
  xSemaphoreGiveRecursive(mcucLock);
}
