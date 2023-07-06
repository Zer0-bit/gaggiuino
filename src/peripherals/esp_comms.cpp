/* 09:32 15/03/2023 - change triggering comment */
#include "esp_comms.h"
#include "pindef.h"
#include "proto/message_converters.h"
#include "proto/profile_converters.h"
#include "proto/settings_converters.h"
#include "proto/proto_serializer.h"
#include "../log.h"

namespace esp {
  McuComms mcuComms;
  bool receivedSettingsAtLeastOnce = false;
  bool receivedProfileAtLeastOnce = false;
  uint32_t requestDataTimer = 0;

  void initialiseState() {
    // Load settings from ESP
    requestDataTimer = 0;
    while (!receivedSettingsAtLeastOnce) {
      if (millis() - requestDataTimer > 1000) {
        espCommsRequestData(McuCommsMessageType::MCUC_DATA_ALL_SETTINGS);
        requestDataTimer = millis();
      }
      espCommsReadData();
      delay(2);
    }
    LOG_INFO("Settings Init");

    // Load profile from ESP
    requestDataTimer = 0;
    while (!receivedProfileAtLeastOnce) {
      if (millis() - requestDataTimer > 1000) {
        espCommsRequestData(McuCommsMessageType::MCUC_DATA_PROFILE);
        requestDataTimer = millis();
      }
      espCommsReadData();
      delay(2);
    }
    LOG_INFO("Profile Init");
  }
}

void handleMessageReceived(McuCommsMessageType messageType, std::vector<uint8_t>& data);
void espCommsInit() {
  USART_ESP.begin(921600);

  // mcuComms.setDebugPort(&USART_ESP);
  esp::mcuComms.begin(USART_ESP, 1000);

  // Set callbacks
  esp::mcuComms.setMessageReceivedCallback(handleMessageReceived);

  esp::initialiseState();
}

//---------------------------------------------------------------------------
//--------------------------- SENDING TO ESP --------------------------------
//---------------------------------------------------------------------------


volatile uint32_t sensorDataTimer = 0;
void espCommsSendSensorData(const SensorState& state, uint32_t frequency) {
  uint32_t now = millis();
  if (now - sensorDataTimer < frequency) return;

  SensorStateSnapshot sensorSnapshot = SensorStateSnapshot {
    .brewActive = state.brewSwitchState,
    .steamActive = state.steamSwitchState,
    .hotWaterSwitchState = state.hotWaterSwitchState,
    .temperature = state.temperature,
    .waterTemperature = state.waterTemperature,
    .pressure = state.smoothedPressure,
    .pumpFlow = state.smoothedPumpFlow,
    .weightFlow = state.smoothedWeightFlow,
    .weight = state.weight,
    .waterLevel = state.waterLvl
  };

  esp::mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_SENSOR_STATE_SNAPSHOT,
    ProtoSerializer::serialize<SensorStateSnapshotConverter>(sensorSnapshot)
  );
  sensorDataTimer = now;
}

volatile uint32_t shotDataTimer;
void espCommsSendShotData(const ShotSnapshot& shotData, uint32_t frequency) {
  uint32_t now = millis();
  if (now - shotDataTimer < frequency) return;

  esp::mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_SHOT_SNAPSHOT,
    ProtoSerializer::serialize<ShotSnapshotConverter>(shotData)
  );
  shotDataTimer = now;
}

void espCommsSendTareScalesCommand() {
  esp::mcuComms.sendMessage(McuCommsMessageType::MCUC_CMD_REMOTE_SCALES_TARE);
}

volatile uint32_t notificationTimer;
void espCommsSendNotification(Notification notification, uint32_t frequency) {
  uint32_t now = millis();
  if (now - notificationTimer < frequency) return;
  notificationTimer = now;

  esp::mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_NOTIFICATION,
    ProtoSerializer::serialize<NotificationConverter>(notification)
  );
}

volatile uint32_t systemStateTimer;
void espCommsSendSystemState(const SystemState& systemState, uint32_t frequency) {
  uint32_t now = millis();
  if (now - systemStateTimer < frequency) return;
  systemStateTimer = now;

  esp::mcuComms.sendMessage(
    McuCommsMessageType::MCUC_DATA_SYSTEM_STATE,
    ProtoSerializer::serialize<SystemStateConverter>(systemState)
  );
}

void espCommsRequestData(McuCommsMessageType dataType) {
  esp::mcuComms.sendMessage(
    McuCommsMessageType::MCUC_REQ_DATA,
    ProtoSerializer::serialize<McuCommsRequestDataConverter>(McuCommsRequestData{ dataType })
  );
}

//---------------------------------------------------------------------------
//------------------------- RECEIVING FROM ESP ------------------------------
//---------------------------------------------------------------------------

void espCommsReadData() {
  esp::mcuComms.readDataAndTick();
}

void handleMessageReceived(McuCommsMessageType messageType, std::vector<uint8_t>& data) {
  switch (messageType) {
  case McuCommsMessageType::MCUC_DATA_PROFILE: {
    Profile profile;
    ProtoSerializer::deserialize<ProfileConverter>(data, profile);
    esp::receivedProfileAtLeastOnce = true;
    onProfileReceived(profile);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_REMOTE_SCALES_WEIGHT: {
    Measurement weight = {};
    ProtoSerializer::deserialize<MeasurementConverter>(data, weight);
    onRemoteScalesWeightReceived(weight.value);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_REMOTE_SCALES_DISCONNECTED: {
    onRemoteScalesDisconnected();
    break;
  }
  case McuCommsMessageType::MCUC_DATA_ALL_SETTINGS: {
    GaggiaSettings settings;
    ProtoSerializer::deserialize<GaggiaSettingsConverter>(data, settings);
    esp::receivedSettingsAtLeastOnce = true;
    onGaggiaSettingsReceived(settings);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_MANUAL_BREW_PHASE: {
    Phase phase;
    ProtoSerializer::deserialize<PhaseConverter>(data, phase);
    onManualBrewPhaseReceived(phase);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_SYSTEM_STATE: {
    UpdateOperationMode updateOperationMode = { OperationMode::BREW_AUTO };
    ProtoSerializer::deserialize<UpdateOperationModeConverter>(data, updateOperationMode);
    onOperationModeReceived(updateOperationMode.operationMode);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_BOILER_SETTINGS: {
    BoilerSettings boilerSettings;
    ProtoSerializer::deserialize<BoilerSettingsConverter>(data, boilerSettings);
    onBoilerSettingsReceived(boilerSettings);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_LED_SETTINGS: {
    LedSettings ledSettings;
    ProtoSerializer::deserialize<LedSettingsConverter>(data, ledSettings);
    onLedSettingsReceived(ledSettings);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_SYSTEM_SETTINGS: {
    SystemSettings systemSettings;
    ProtoSerializer::deserialize<SystemSettingsConverter>(data, systemSettings);
    onSystemSettingsReceived(systemSettings);
    break;
  }
  case McuCommsMessageType::MCUC_DATA_BREW_SETTINGS: {
    BrewSettings brewSettings;
    ProtoSerializer::deserialize<BrewSettingsConverter>(data, brewSettings);
    onBrewSettingsReceived(brewSettings);
    break;
  }
  case McuCommsMessageType::MCUC_CMD_UPDATE_OPERATION_MODE: {
    UpdateOperationMode command{ .operationMode = OperationMode::BREW_AUTO }; // init default
    ProtoSerializer::deserialize<UpdateOperationModeConverter>(data, command);
    onOperationModeReceived(command.operationMode);
    break;
  }
  default: // Ignore message in all other cases
    break;
  }
}
