/* 09:32 15/03/2023 - change triggering comment */
#ifndef ESP_COMMS_H
#define ESP_COMMS_H

#include "mcu_comms.h"

void espCommsInit();

void espCommsReadData();

void espCommsSendSensorData(const SensorState& state, uint32_t frequency = 1000);
void espCommsSendShotData(ShotSnapshot& shotData, uint32_t frequency = 100);

void onProfileReceived(Profile& profile);

#endif
