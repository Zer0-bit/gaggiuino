#ifndef JSON_SETTINGS_CONVERTERS_H
#define JSON_SETTINGS_CONVERTERS_H

#include "ArduinoJson.h"
#include "gaggia_settings.h"

namespace json {
  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  void mapLedSettingsToJson(const LedSettings& led, JsonObject& target);
  void mapBoilerSettingsToJson(const BoilerSettings& boiler, JsonObject& target);
  void mapSystemSettingsToJson(const SystemSettings& system, JsonObject& target);
  void mapBrewSettingsToJson(const BrewSettings& brew, JsonObject& target);
  void mapAllSettingsToJson(const GaggiaSettings& settings, JsonObject& target);
  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  SystemSettings mapJsonToSystemSettings(const JsonObject& json);
  LedSettings mapJsonToLedSettings(const JsonObject& json);
  BrewSettings mapJsonToBrewSettings(const JsonObject& json);
  BoilerSettings mapJsonToBoilerSettings(const JsonObject& json);
  GaggiaSettings mapJsonToAllSettings(const JsonObject& json);
}
#endif
