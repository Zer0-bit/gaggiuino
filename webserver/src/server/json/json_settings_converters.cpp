#include "json_settings_converters.h"

namespace json {

  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  void mapLedSettingsToJson(const LedSettings& led, JsonObject& target) {
    target["state"] = led.state;
    target["disco"] = led.disco;
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
    target["pumpFlowAtZero"] = system.pumpFlowAtZero;
  }

  void mapBrewSettingsToJson(const BrewSettings& brew, JsonObject& target) {
    target["basketPrefill"] = brew.basketPrefill;
    target["brewDeltaState"] = brew.brewDeltaState;
    target["homeOnShotFinish"] = brew.homeOnShotFinish;
  }

  void mapScalesSettingsToJson(const ScalesSettings& scales, JsonObject& target) {
    target["forcePredictive"] = scales.forcePredictive;
    target["hwScalesEnabled"] = scales.hwScalesEnabled;
    target["hwScalesF1"] = scales.hwScalesF1;
    target["hwScalesF2"] = scales.hwScalesF2;
    target["btScalesEnabled"] = scales.btScalesEnabled;
    target["btScalesAutoConnect"] = scales.btScalesAutoConnect;
  }

  void mapAllSettingsToJson(const GaggiaSettings& settings, JsonObject& target) {
    JsonObject systemObj = target.createNestedObject("system");
    JsonObject boilerObj = target.createNestedObject("boiler");
    JsonObject brewObj = target.createNestedObject("brew");
    JsonObject ledObj = target.createNestedObject("led");
    JsonObject scalesObj = target.createNestedObject("scales");
    mapSystemSettingsToJson(settings.system, systemObj);
    mapBoilerSettingsToJson(settings.boiler, boilerObj);
    mapBrewSettingsToJson(settings.brew, brewObj);
    mapLedSettingsToJson(settings.led, ledObj);
    mapScalesSettingsToJson(settings.scales, scalesObj);
  };

  void mapBleScalesToJson(const blescales::Scales& scales, JsonObject& target) {
    target["name"] = scales.name;
    target["address"] = scales.address;
  }

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  SystemSettings mapJsonToSystemSettings(const JsonObject& json) {
    return SystemSettings{
      .pumpFlowAtZero = json["pumpFlowAtZero"],
      .lcdSleep = json["lcdSleep"],
      .warmupState = json["warmupState"],
    };
  }

  LedSettings mapJsonToLedSettings(const JsonObject& json) {
    JsonObject colorObject = json["color"].as<JsonObject>();
    return LedSettings{
      .state = json["state"],
      .disco = json["disco"],
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

  ScalesSettings mapJsonToScalesSettings(const JsonObject& json) {
    return ScalesSettings{
      .forcePredictive = json["forcePredictive"],
      .hwScalesEnabled = json["hwScalesEnabled"],
      .hwScalesF1 = json["hwScalesF1"],
      .hwScalesF2 = json["hwScalesF2"],
      .btScalesEnabled = json["btScalesEnabled"],
      .btScalesAutoConnect = json["btScalesAutoConnect"],
    };
  }

  GaggiaSettings mapJsonToAllSettings(const JsonObject& json) {
    JsonObject boilerJson = json["boiler"].as<JsonObject>();
    JsonObject systemJson = json["system"].as<JsonObject>();
    JsonObject brewJson = json["brew"].as<JsonObject>();
    JsonObject ledJson = json["led"].as<JsonObject>();
    JsonObject scalesJson = json["scales"].as<JsonObject>();
    return GaggiaSettings{
      .boiler = mapJsonToBoilerSettings(boilerJson),
      .system = mapJsonToSystemSettings(systemJson),
      .brew = mapJsonToBrewSettings(brewJson),
      .led = mapJsonToLedSettings(ledJson),
      .scales = mapJsonToScalesSettings(scalesJson),
    };
  }

  blescales::Scales mapJsonToBleScales(const JsonObject& json) {
    return blescales::Scales{
      .name = json["name"],
      .address = json["address"],
    };
  }

}
