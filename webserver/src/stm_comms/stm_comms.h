#ifndef STM_COMMS_H
#define STM_COMMS_H

#include "mcu_comms.h"

McuComms mcuComms;

void onSensorStateSnapshotReceived(SensorStateSnapshot& snapshot);
void onShotSnapshotReceived(ShotSnapshot& snapshot);

void stmCommsInit(HardwareSerial& serial) {
  serial.setRxBufferSize(256);
  serial.setTxBufferSize(256);
  serial.begin(460800);

  // mcuComms.setDebugPort(&Serial);
  mcuComms.begin(serial);

  // Set callbacks
  mcuComms.setShotSnapshotCallback(onShotSnapshotReceived);
  mcuComms.setSensorStateSnapshotCallback(onSensorStateSnapshotReceived);
}

void stmCommsReadData() {
  mcuComms.readDataAndTick();
}

void stmCommsSendWeight(float weight) {
  mcuComms.sendWeight(weight);
}

#endif
