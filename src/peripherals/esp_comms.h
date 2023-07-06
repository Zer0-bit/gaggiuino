/* 09:32 15/03/2023 - change triggering comment */
#ifndef ESP_COMMS_H
#define ESP_COMMS_H

#include "mcu_comms.h"
#include "gaggia_settings.h"
#include "system_state.h"
#include "notification_message.h"

void espCommsInit();

void espCommsReadData();

// Get temp stored values
Phase& espCommsGetManualPhase();

// Send to MCU
void espCommsSendSensorData(const SensorState& state, uint32_t frequency = 1000);
void espCommsSendShotData(const ShotSnapshot& shotData, uint32_t frequency = 100);
void espCommsSendTareScalesCommand();
void espCommsSendNotification(const Notification notification, uint32_t frequency = 1000);
void espCommsSendSystemState(const SystemState& systemState, uint32_t frequency = 1000);
void espCommsRequestData(McuCommsMessageType dataType);

// MCU updates
void onRemoteScalesWeightReceived(float weight);
void onRemoteScalesDisconnected();

// Settings updates
void onGaggiaSettingsReceived(const GaggiaSettings& gaggiaSettings);
void onBoilerSettingsReceived(const BoilerSettings& boilerSettings);
void onLedSettingsReceived(const LedSettings& ledSettings);
void onSystemSettingsReceived(const SystemSettings& systemSettings);
void onBrewSettingsReceived(const BrewSettings& brewSettings);

// System state updates
void onOperationModeReceived(const OperationMode operationMode);

// Profiling
void onProfileReceived(const Profile& profile);
void onManualBrewPhaseReceived(const Phase& phase);

#endif
