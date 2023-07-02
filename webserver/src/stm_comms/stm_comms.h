#ifndef STM_COMMS_H
#define STM_COMMS_H

#include "mcu_comms.h"

void stmCommsInit(HardwareSerial& serial);
void stmCommsReadData();
void stmCommsSendWeight(float weight);
void stmCommsSendScaleDisconnected();

// To be defined elsewhere
void onSensorStateSnapshotReceived(SensorStateSnapshot& snapshot);
void onShotSnapshotReceived(ShotSnapshot& snapshot);
void onScalesTareReceived();

#endif
