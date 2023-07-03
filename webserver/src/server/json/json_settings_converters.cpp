#include "json_settings_converters.h"

namespace json {

  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  void mapLedSettingsToJson(const LedSettings& led, JsonObject& target) {
    target["state"] = led.state;
    JsonObject ledColor = target.createNestedObject("color");
    ledColor["R"] = led.color.R;
    ledColor["G"] = led.color.G;
    ledColor["B"] = led.color.B;
  }

  void mapBoilerSettingsToJson(const BoilerSettings& boiler, JsonObject& target) {
    target["brewDivider"] = boiler.brewDivider;
    target["hpwr"] = boiler.hpwr;
    target["mainDivider"] = boiler.mainDivider;
    target["offsetTemp"] = boiler.offsetTemp;
    target["steamSetPoint"] = boiler.steamSetPoint;
  }

  void mapSystemSettingsToJson(const SystemSettings& system, JsonObject& target) {
    target["warmupState"] = system.warmupState;
    target["lcdSleep"] = system.lcdSleep;
    target["powerLineFrequency"] = system.powerLineFrequency;
    target["pumpFlowAtZero"] = system.pumpFlowAtZero;
    target["scalesF1"] = system.scalesF1;
    target["scalesF2"] = system.scalesF2;
  }

  void mapBrewSettingsToJson(const BrewSettings& brew, JsonObject& target) {
    target["basketPrefill"] = brew.basketPrefill;
    target["brewDeltaState"] = brew.brewDeltaState;
    target["homeOnShotFinish"] = brew.homeOnShotFinish;
  }

  void mapAllSettingsToJson(const GaggiaSettings& settings, JsonObject& target) {
    JsonObject systemObj = target.createNestedObject("system");
    JsonObject boilerObj = target.createNestedObject("boiler");
    JsonObject brewObj = target.createNestedObject("brew");
    JsonObject ledObj = target.createNestedObject("led");
    mapSystemSettingsToJson(settings.system, systemObj);
    mapBoilerSettingsToJson(settings.boiler, boilerObj);
    mapBrewSettingsToJson(settings.brew, brewObj);
    mapLedSettingsToJson(settings.led, ledObj);
  };

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  SystemSettings mapJsonToSystemSettings(const JsonObject& json) {
    return SystemSettings{
      .powerLineFrequency = json["powerLineFrequency"],
      .pumpFlowAtZero = json["pumpFlowAtZero"],
      .scalesF1 = json["scalesF1"],
      .scalesF2 = json["scalesF2"],
      .lcdSleep = json["lcdSleep"],
      .warmupState = json["warmupState"],
    };
  }

  LedSettings mapJsonToLedSettings(const JsonObject& json) {
    JsonObject colorObject = json["color"].as<JsonObject>();
    return LedSettings{
      .state = json["state"],
      .color = LedSettings::Color {
        .R = colorObject["R"],
        .G = colorObject["G"],
        .B = colorObject["B"],
      }
    };
  }

  BrewSettings mapJsonToBrewSettings(const JsonObject& json) {
    return BrewSettings{
      .basketPrefill = json["basketPrefill"],
      .homeOnShotFinish = json["homeOnShotFinish"],
      .brewDeltaState = json["brewDeltaState"],
    };
  }

  BoilerSettings mapJsonToBoilerSettings(const JsonObject& json) {
    return BoilerSettings{
      .steamSetPoint = json["steamSetPoint"],
      .offsetTemp = json["offsetTemp"],
      .hpwr = json["hpwr"],
      .mainDivider = json["mainDivider"],
      .brewDivider = json["brewDivider"],
    };
  }

  GaggiaSettings mapJsonToAllSettings(const JsonObject& json) {
    JsonObject boilerJson = json["boiler"].as<JsonObject>();
    JsonObject systemJson = json["system"].as<JsonObject>();
    JsonObject brewJson = json["brew"].as<JsonObject>();
    JsonObject ledJson = json["led"].as<JsonObject>();
    return GaggiaSettings{
      .boiler = mapJsonToBoilerSettings(boilerJson),
      .system = mapJsonToSystemSettings(systemJson),
      .brew = mapJsonToBrewSettings(brewJson),
      .led = mapJsonToLedSettings(ledJson),
    };
  }
}
