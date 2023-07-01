#ifndef STM_COMMS_H
#define STM_COMMS_H

#include "mcu_comms.h"
#include "gaggia_settings.h"

void stmCommsInit(HardwareSerial& serial);
void stmCommsReadData();
void stmCommsSendWeight(float weight);
void stmCommsSendScaleDisconnected();
void stmCommsSendGaggiaSettings(const GaggiaSettings& settings);
void stmCommsSendProfile(const Profile& profile);

// To be defined elsewhere
void onSensorStateSnapshotReceived(const SensorStateSnapshot& snapshot);
void onShotSnapshotReceived(const ShotSnapshot& snapshot);
void onScalesTareReceived();
void onGaggiaSettingsRequested();
void onProfileRequested();

#endif
