#include "ble_scales.h"
#include <memory>
#include "remote_scales.h"
#include "scales/acaia.h"
#include "../stm_comms/stm_comms.h"
#include "../log/log.h"
#include "../task_config.h"

namespace {
  RemoteScalesScanner remoteScalesScanner;
  std::unique_ptr<RemoteScales> bleScales;
}

void bleScalesTask(void* params);

void bleScalesInit() {
  AcaiaScalesPlugin::apply();
  BLEDevice::init("Gaggiuino");

  xTaskCreateUniversal(bleScalesTask, "bleScales", configMINIMAL_STACK_SIZE + 4096, NULL, PRIORITY_BLE_SCALES_MAINTAINANCE, NULL, CORE_BLE_SCALES_MAINTAINANCE);
}

void bleScalesTask(void* params) {
  remoteScalesScanner.initializeAsyncScan();

  LOG_INFO("Remote scales and bluetooth initialized");

  for (;;) {
    bleScalesMaintainConnection();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void bleScalesMaintainConnection() {
  if (bleScales.get() == nullptr) { // No scale discovered yet. Keep checking scan results to find scales.
    std::vector<RemoteScales*> scales = remoteScalesScanner.getDiscoveredScales();

    if (scales.size() > 0) {
      LOG_INFO("We have %d discovered scales.", scales.size());
      bleScales.reset(scales[0]);
      remoteScalesScanner.stopAsyncScan();
      bleScales->setWeightUpdatedCallback(stmCommsSendWeight);
      bleScales->setLogCallback([](std::string message) { LOG_INFO(message.c_str()); });
      bleScales->connect();
    }
  }
  else if (!bleScales->isConnected()) { // Scale discovered but not connected. Make sure it's still reachable.
    LOG_INFO("Connection failed. Will retry.");
    stmCommsSendScaleDisconnected();
    bleScales.release();
    remoteScalesScanner.restartAsyncScan();
  }
  else if (bleScales->isConnected()) { // Scale stil connected. Invoke update to keep alive.
    bleScales->update();
  }
}

void bleScalesTare() {
  if (bleScales.get() != nullptr) {
    bleScales->tare();
  }
}
