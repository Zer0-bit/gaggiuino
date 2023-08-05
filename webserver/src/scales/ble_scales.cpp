#include "ble_scales.h"
#include <memory>
#include "remote_scales.h"
#include "scales/acaia.h"
#include "../state/state.h"
#include "../log/log.h"
#include "../task_config.h"
#include <algorithm>

namespace blescales {
  RemoteScalesScanner remoteScalesScanner;
  std::unique_ptr<RemoteScales> bleScales;

  void bleScalesTask(void* params);

  // ----------------------------------------------------------------
  // ------------------------- PUBLIC METHODS -----------------------
  // ----------------------------------------------------------------

  void init() {
    AcaiaScalesPlugin::apply();
    xTaskCreateUniversal(bleScalesTask, "bleScales", configMINIMAL_STACK_SIZE + 4096, NULL, PRIORITY_BLE_SCALES_MAINTAINANCE, NULL, CORE_BLE_SCALES_MAINTAINANCE);
  }

  void tare() {
    if (bleScales.get() != nullptr) {
      bleScales->tare();
    }
  }

  std::vector<Scales> getAvailableScales() {
    auto discoveredScales = remoteScalesScanner.getDiscoveredScales();
    std::vector<Scales> result(discoveredScales.size());

    std::transform(discoveredScales.begin(), discoveredScales.end(), result.begin(), [](RemoteScales* input) {
      return Scales{ .name = input->getDeviceName(), .address = input->getDeviceAddress() };
      });

    return result;
  }

  Scales getConnectedScales() {
    if (bleScales.get() != nullptr && bleScales->isConnected()) {
      return { .name = bleScales->getDeviceName(), .address = bleScales->getDeviceAddress() };
    }
    return { .name = "", .address = "" };
  }

  // ----------------------------------------------------------------
  // ---------------------- PRIVATE HELPER METHODS ------------------
  // ----------------------------------------------------------------

  void handleBleDevice();
  void maintainConnection();

  void bleScalesTask(void* params) {
    LOG_INFO("Remote scales and bluetooth initialized");

    for (;;) {
      handleBleDevice();
      maintainConnection();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }

  void handleBleDevice() {
    if (state::getSettings().scales.btScalesEnabled && !BLEDevice::getInitialized()) {
      BLEDevice::init("Gaggiuino");
    }

    if (!state::getSettings().scales.btScalesEnabled && BLEDevice::getInitialized()) {
      remoteScalesScanner.stopAsyncScan();
      if (bleScales.get() != nullptr && bleScales->isConnected()) {
        bleScales->disconnect();
        bleScales.release();
        state::updateConnectedScales({});
      }
      BLEDevice::deinit();
    }
  }

  void maintainConnection() {
    if (!state::getSettings().scales.btScalesEnabled) {
      return;
    }

    if (bleScales.get() == nullptr) { // No scale discovered yet. Keep checking scan results to find scales.
      remoteScalesScanner.initializeAsyncScan();

      std::vector<RemoteScales*> scales = remoteScalesScanner.getDiscoveredScales();

      if (scales.size() > 0) {
        LOG_INFO("We have discovered %d matching scales.", scales.size());
        bleScales.reset(scales[0]);
        bleScales->setWeightUpdatedCallback(onWeightReceived);
        bleScales->setLogCallback([](std::string message) { LOG_INFO(message.c_str()); });
        bleScales->connect();
        state::updateConnectedScales({ bleScales->getDeviceName(), bleScales->getDeviceAddress() });
      }
    }
    else if (!bleScales->isConnected()) { // Scale discovered but not connected. Make sure it's still reachable.
      LOG_INFO("Connection failed. Will retry.");
      remoteScalesScanner.stopAsyncScan();
      bleScales.release();
      state::updateConnectedScales({});
    }
    else if (bleScales->isConnected()) { // Scale stil connected. Invoke update to keep alive.
      remoteScalesScanner.stopAsyncScan();
      bleScales->update();
    }
  }
}
