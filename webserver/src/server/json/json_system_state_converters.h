#ifndef JSON_SYSTEM_STATE_CONVERTERS_H
#define JSON_SYSTEM_STATE_CONVERTERS_H

#include "ArduinoJson.h"
#include "system_state.h"
#include "sensors_state.h"
#include "string"

namespace json {
  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  std::string mapOperationModeToJsonValue(const OperationMode& mode);
  void mapSystemStateToJson(const SystemState& systemState, JsonObject& target);
  void mapUpdateSystemStateCommmandToJson(const UpdateSystemStateComand& command, JsonObject& target);
  void mapSensorStateToJson(const SensorStateSnapshot& sensorState, JsonObject& target);
  void mapShotSnapshotToJson(const ShotSnapshot& shotSnaposhot, JsonObject& target);
  std::string mapDescalingStateToJsonValue(const DescalingState& state);
  void mapDescalingProgressToJson(const DescalingProgress& progress, JsonObject& target);

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  OperationMode mapJsonValueToOperationMode(const std::string& modeValue);
  SystemState mapJsonToSystemState(const JsonObject& json);
  UpdateSystemStateComand mapJsonToUpdateSystemStateCommand(const JsonObject& json);
}
#endif
