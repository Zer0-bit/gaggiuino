#include "json_system_state_converters.h"

namespace json {

  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  void mapSystemStateToJson(const SystemState& systemState, JsonObject& target) {
    target["startupInitFinished"] = systemState.startupInitFinished;
    target["operationMode"] = mapOperationModeToJsonValue(systemState.operationMode);
    target["tofReady"] = systemState.tofReady;
    target["isSteamForgottenON"] = systemState.isSteamForgottenON;
    target["scalesPresent"] = systemState.scalesPresent;
    target["timeAlive"] = systemState.timeAlive;
    target["descaleProgress"] = systemState.descaleProgress;
  }

  std::string mapOperationModeToJsonValue(const OperationMode& mode) {
    switch (mode) {
    case OperationMode::BREW_AUTO: return "BREW_AUTO";
    case OperationMode::BREW_MANUAL: return "BREW_MANUAL";
    case OperationMode::FLUSH: return "FLUSH";
    case OperationMode::DESCALE: return "DESCALE";
    case OperationMode::STEAM: return "STEAM";
    default: return "BREW_AUTO";
    }
  }

  void mapUpdateOperationModeToJson(const UpdateOperationMode& updateOperationMode, JsonObject& target) {
    target["operationMode"] = mapOperationModeToJsonValue(updateOperationMode.operationMode);
  }

  void mapSensorStateToJson(const SensorStateSnapshot& sensorState, JsonObject& target) {
    target["brewActive"] = sensorState.brewActive;
    target["steamActive"] = sensorState.steamActive;
    target["hotWaterActive"] = sensorState.hotWaterSwitchState;
    target["temperature"] = sensorState.temperature;
    target["waterTemperature"] = sensorState.waterTemperature;
    target["pressure"] = sensorState.pressure;
    target["pumpFlow"] = sensorState.pumpFlow;
    target["weightFlow"] = sensorState.weightFlow;
    target["weight"] = sensorState.weight;
    target["waterLevel"] = sensorState.waterLevel;
  }

  void mapShotSnapshotToJson(const ShotSnapshot& shotSnaposhot, JsonObject& target) {
    target["timeInShot"] = shotSnaposhot.timeInShot;
    target["pressure"] = shotSnaposhot.pressure;
    target["pumpFlow"] = shotSnaposhot.pumpFlow;
    target["weightFlow"] = shotSnaposhot.weightFlow;
    target["temperature"] = shotSnaposhot.temperature;
    target["shotWeight"] = shotSnaposhot.shotWeight;
    target["waterPumped"] = shotSnaposhot.waterPumped;
    target["targetTemperature"] = shotSnaposhot.targetTemperature;
    target["targetPumpFlow"] = shotSnaposhot.targetPumpFlow;
    target["targetPressure"] = shotSnaposhot.targetPressure;
  }

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  OperationMode mapJsonValueToOperationMode(const std::string& modeValue) {
    if (modeValue == "BREW_AUTO") return OperationMode::BREW_AUTO;
    if (modeValue == "BREW_MANUAL") return OperationMode::BREW_MANUAL;
    if (modeValue == "FLUSH") return OperationMode::FLUSH;
    if (modeValue == "DESCALE") return OperationMode::DESCALE;
    if (modeValue == "STEAM") return OperationMode::STEAM;
    return OperationMode::BREW_AUTO;
  }

  UpdateOperationMode mapJsonToUpdateOperationMode(const JsonObject& json) {
    return UpdateOperationMode{
      .operationMode = mapJsonValueToOperationMode(json["operationMode"]),
    };
  }

  SystemState mapJsonToSystemState(const JsonObject& json) {
    SystemState systemState;
    systemState.startupInitFinished = json["startupInitFinished"];
    systemState.operationMode = mapJsonValueToOperationMode(json["operationMode"]);
    systemState.tofReady = json["tofReady"];
    systemState.isSteamForgottenON = json["isSteamForgottenON"];
    systemState.scalesPresent = json["scalesPresent"];
    systemState.timeAlive = json["timeAlive"];
    systemState.descaleProgress = json["descaleProgress"];
    return systemState;
  }
}
