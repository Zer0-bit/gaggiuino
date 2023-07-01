/* 09:32 15/03/2023 - change triggering comment */
#include "esp_comms.h"
#include "pindef.h"
#include "proto/message_converters.h"
#include "proto/profile_converters.h"
#include "proto/settings_converters.h"
#include "proto/proto_serializer.h"

namespace {
  class McuCommsSingleton {
  public:
    static McuComms& getInstance() {
      static McuComms instance;
      return instance;
    }
  private:
    McuCommsSingleton() = default;
    ~McuCommsSingleton() = default;
  };
}

void handleMessageReceived(McuCommsMessageType messageType, std::vector<uint8_t>& data);
void espCommsInit() {
  USART_ESP.begin(460800);

  // mcuComms.setDebugPort(&USART_ESP);
  McuCommsSingleton::getInstance().begin(USART_ESP);

  // Set callbacks
  McuCommsSingleton::getInstance().setMessageReceivedCallback(handleMessageReceived);
}

//---------------------------------------------------------------------------
//--------------------------- SENDING TO ESP --------------------------------
//---------------------------------------------------------------------------

volatile uint32_t sensorDataTimer = 0;
void espCommsSendSensorData(const SensorState& state, uint32_t frequency) {
  uint32_t now = millis();
  if (now - sensorDataTimer < frequency) return;

  SensorStateSnapshot sensorSnapshot = SensorStateSnapshot{
    .brewActive = state.brewSwitchState,
    .steamActive = state.steamSwitchState,
    .hotWaterSwitchState = state.hotWaterSwitchState,
    .temperature = state.temperature,
    .waterTemperature = state.waterTemperature,
    .pressure = state.smoothedPressure,
    .pumpFlow = state.smoothedPumpFlow,
    .weightFlow = state.smoothedWeightFlow,
    .weight = state.weight,
    .waterLevel = state.waterLvl,
  };

  McuCommsSingleton::getInstance().sendMessage(
    McuCommsMessageType::MCUC_DATA_SENSOR_STATE_SNAPSHOT,
    ProtoSerializer::serialize<SensorStateSnapshotConverter>(sensorSnapshot)
  );
  sensorDataTimer = now;
}

volatile uint32_t shotDataTimer;
void espCommsSendShotData(const ShotSnapshot& shotData, uint32_t frequency) {
  uint32_t now = millis();
  if (now - shotDataTimer < frequency) return;

  McuCommsSingleton::getInstance().sendMessage(
    McuCommsMessageType::MCUC_DATA_SHOT_SNAPSHOT,
    ProtoSerializer::serialize<ShotSnapshotConverter>(shotData)
  );
  shotDataTimer = now;
}

void espCommsSendTareScalesCommand() {
  McuCommsSingleton::getInstance().sendMessage(McuCommsMessageType::MCUC_CMD_REMOTE_SCALES_TARE);
}

void espCommsSendNotification(Notification notification) {
  McuCommsSingleton::getInstance().sendMessage(
    McuCommsMessageType::MCUC_DATA_NOTIFICATION,
    ProtoSerializer::serialize<NotificationConverter>(notification)
  );
}

volatile uint32_t systemStateTimer;
void espCommsSendSystemState(const SystemState& systemState, uint32_t frequency) {
  uint32_t now = millis();
  if (now - systemStateTimer < frequency) return;

  McuCommsSingleton::getInstance().sendMessage(
    McuCommsMessageType::MCUC_DATA_SYSTEM_STATE,
    ProtoSerializer::serialize<SystemStateConverter>(systemState)
  );

  systemStateTimer = now;
}

void espCommsRequestData(McuCommsMessageType dataType) {
  McuCommsSingleton::getInstance().sendMessage(
    McuCommsMessageType::MCUC_REQ_DATA,
    ProtoSerializer::serialize<McuCommsRequestDataConverter>(McuCommsRequestData{dataType})
  );
}

//---------------------------------------------------------------------------
//------------------------- RECEIVING FROM ESP ------------------------------
//---------------------------------------------------------------------------
void espCommsReadData() {
  McuCommsSingleton::getInstance().readDataAndTick();
}

void handleMessageReceived(McuCommsMessageType messageType, std::vector<uint8_t>& data) {
  switch (messageType) {
  case McuCommsMessageType::MCUC_DATA_PROFILE: {
    Profile profile;
    ProtoSerializer::deserialize<ProfileConverter>(data, profile);
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
  default: // Ignore message in all other cases
    break;
  }
}
