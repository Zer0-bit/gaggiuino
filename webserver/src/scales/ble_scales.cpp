#include "ble_scales.h"
#include <memory>
#include "remote_scales.h"
#include "scales/acaia.h"
#include "../stm_comms/stm_comms.h"

namespace {
  const uint16_t BLE_CONNECTION_MAINTENANCE_TIME = 200;
  uint32_t bleLastMaintenanceTime = 0;

  RemoteScalesScanner remoteScalesScanner;
  std::unique_ptr<RemoteScales> bleScales;
}

void bleScalesInit() {
  AcaiaScalesPlugin::apply();
  BLEDevice::init("Gaggiuino");
  remoteScalesScanner.initializeAsyncScan();
}

void bleScalesMaintainConnection() {
  uint32_t now = millis();
  if (now - bleLastMaintenanceTime < BLE_CONNECTION_MAINTENANCE_TIME) {
    return;
  }

  bleLastMaintenanceTime = now;

  if (bleScales.get() == nullptr) { // No scale discovered yet. Keep checking scan results to find scales.
    std::vector<RemoteScales*> scales = remoteScalesScanner.getDiscoveredScales();
    Serial.printf("We have %d discovered scales.\n", scales.size());

    if (scales.size() > 0) {
      bleScales.reset(scales[0]);
      remoteScalesScanner.stopAsyncScan();
      bleScales->setWeightUpdatedCallback(stmCommsSendWeight);
      bleScales->setDebugPort(&Serial);
      bleScales->connect();
    }
  }
  else if (!bleScales->isConnected()) { // Scale discovered but not connected. Make sure it's still reachable.
    Serial.println("Connection failed. Will retry.");
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
