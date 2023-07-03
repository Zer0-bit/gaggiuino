#ifndef JSON_SYSTEM_STATE_CONVERTERS_H
#define JSON_SYSTEM_STATE_CONVERTERS_H

#include "ArduinoJson.h"
#include "system_state.h"
#include "string"

namespace json {
  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  std::string mapOperationModeToJsonValue(const OperationMode& mode);
  void mapSystemStateToJson(const SystemState& systemState, JsonObject& target);
  void mapUpdateOperationModeToJson(const UpdateOperationMode& updateOperationMode, JsonObject& target);

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  OperationMode mapJsonValueToOperationMode(const std::string& modeValue);
  SystemState mapJsonToSystemState(const JsonObject& json);
  UpdateOperationMode mapJsonToUpdateOperationMode(const JsonObject& json);
}
#endif
