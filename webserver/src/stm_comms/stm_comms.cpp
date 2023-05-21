#include "stm_comms.h"

namespace {
  McuComms mcuComms;
}

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
}

void stmCommsReadData() {
  mcuComms.readDataAndTick();
}

void stmCommsSendWeight(float weight) {
  mcuComms.sendRemoteScalesWeight(weight);
}

void stmCommsSendScaleDisconnected() {
  mcuComms.sendRemoteScalesDisconnected();
}
