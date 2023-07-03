#include "json_profile_converters.h"
#include "string"

#include "../../state/state.h"

namespace json {

  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  std::string mapTransitionCurveToJsonValue(const TransitionCurve curve) {
    switch (curve) {
    case TransitionCurve::EASE_IN_OUT: return "EASE_IN_OUT";
    case TransitionCurve::EASE_IN: return "EASE_IN";
    case TransitionCurve::EASE_OUT: return "EASE_OUT";
    case TransitionCurve::LINEAR: return "LINEAR";
    case TransitionCurve::INSTANT: return "INSTANT";
    default: return "EASE_IN_OUT";
    }
  }

  std::string mapPhaseTypeToJsonValue(const PhaseType phaseType) {
    switch (phaseType) {
    case PhaseType::PRESSURE: return "PRESSURE";
    case PhaseType::FLOW: return "FLOW";
    default: return "PRESSURE";
    }
  }

  void addSkippable(JsonObject& target, const char* key, uint32_t value) {
    if (value != 0) {
      target[key] = value;
    }
  }

  void addSkippable(JsonObject& target, const char* key, float value) {
    if (value != 0) {
      target[key] = value;
    }
  }

  void addSkippable(JsonObject& target, const char* key, std::string value) {
    if (value.size() != 0) {
      target[key] = value;
    }
  }

  void mapRecipeToJson(const BrewRecipe& recipe, JsonObject& target) {
    addSkippable(target, "coffeeIn", recipe.coffeeIn);
    addSkippable(target, "coffeeOut", recipe.coffeeOut);
    addSkippable(target, "ratio", recipe.ratio);
  }

  void mapTransitionToJson(const Transition& transition, JsonObject& target) {
    addSkippable(target, "start", transition.start);
    addSkippable(target, "end", transition.end);
    target["curve"] = mapTransitionCurveToJsonValue(transition.curve);
    addSkippable(target, "time", transition.time);
  }

  void mapPhaseStopConditionsToJson(const PhaseStopConditions& stopConditions, JsonObject& target) {
    addSkippable(target, "time", stopConditions.time);
    addSkippable(target, "pressureAbove", stopConditions.pressureAbove);
    addSkippable(target, "pressureBelow", stopConditions.pressureBelow);
    addSkippable(target, "flowAbove", stopConditions.flowAbove);
    addSkippable(target, "flowBelow", stopConditions.flowBelow);
    addSkippable(target, "weight", stopConditions.weight);
    addSkippable(target, "waterPumpedInPhase", stopConditions.waterPumpedInPhase);
  }

  void mapPhaseToJson(const Phase& phase, JsonObject& target) {
    auto targetJson = target.createNestedObject("target");
    auto stopConditionsJson = target.createNestedObject("stopConditions");

    target["type"] = mapPhaseTypeToJsonValue(phase.type);
    target["skip"] = phase.skip;
    mapTransitionToJson(phase.target, targetJson);
    mapPhaseStopConditionsToJson(phase.stopConditions, stopConditionsJson);
    addSkippable(target, "name", phase.name);
    addSkippable(target, "restriction", phase.restriction);
    addSkippable(target, "waterTemperature", phase.waterTemperature);
  }

  void mapGlobalStopConditionsToJson(const GlobalStopConditions& globalStopConditions, JsonObject& target) {
    addSkippable(target, "time", globalStopConditions.time);
    addSkippable(target, "weight", globalStopConditions.weight);
    addSkippable(target, "waterPumped", globalStopConditions.waterPumped);
  }

  void mapProfileToJson(ProfileId id, const Profile& profile, JsonObject& target) {
    target["id"] = id;
    target["name"] = profile.name;
    auto phasesJson = target.createNestedArray("phases");
    for (auto& phase : profile.phases) {
      auto phaseJson = phasesJson.createNestedObject();
      mapPhaseToJson(phase, phaseJson);
    }
    auto globalStopConditionsJson = target.createNestedObject("globalStopConditions");
    mapGlobalStopConditionsToJson(profile.globalStopConditions, globalStopConditionsJson);
    target["waterTemperature"] = profile.waterTemperature;
    auto recipeJson = target.createNestedObject("recipe");
    mapRecipeToJson(profile.recipe, recipeJson);
  }

  void mapProfileSummaryToJson(const SavedProfile& summary, JsonObject& target) {
    target["id"] = summary.id;
    target["name"] = summary.name;
    target["active"] = state::getActiveProfileId() == summary.id;
  }

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  TransitionCurve mapJsonValueToTransitionCurve(const std::string& curve) {
    if (curve == "EASE_IN_OUT") return TransitionCurve::EASE_IN_OUT;
    if (curve == "EASE_IN") return TransitionCurve::EASE_IN;
    if (curve == "EASE_OUT") return TransitionCurve::EASE_OUT;
    if (curve == "LINEAR") return TransitionCurve::LINEAR;
    if (curve == "INSTANT") return TransitionCurve::INSTANT;
    return TransitionCurve::EASE_IN_OUT;
  }

  PhaseType mapJsonValueToPhaseType(const std::string& phaseType) {
    if (phaseType == "PRESSURE") return PhaseType::PRESSURE;
    if (phaseType == "FLOW") return PhaseType::FLOW;
    return PhaseType::PRESSURE;
  }

  BrewRecipe mapJsonToRecipe(const JsonObject& source) {
    BrewRecipe recipe;
    recipe.coffeeIn = source["coffeeIn"];
    recipe.coffeeOut = source["coffeeOut"];
    recipe.ratio = source["ratio"];
    return recipe;
  }

  Transition mapJsonToTransition(const JsonObject& source) {
    Transition transition;
    transition.start = source["start"];
    transition.end = source["end"];
    transition.curve = mapJsonValueToTransitionCurve(source["curve"]);
    transition.time = source["time"];
    return transition;
  }

  PhaseStopConditions mapJsonToPhaseStopConditions(const JsonObject& source) {
    PhaseStopConditions stopConditions;
    stopConditions.time = source["time"];
    stopConditions.pressureAbove = source["pressureAbove"];
    stopConditions.pressureBelow = source["pressureBelow"];
    stopConditions.flowAbove = source["flowAbove"];
    stopConditions.flowBelow = source["flowBelow"];
    stopConditions.weight = source["weight"];
    stopConditions.waterPumpedInPhase = source["waterPumpedInPhase"];
    return stopConditions;
  }

  Phase mapJsonToPhase(const JsonObject& source) {
    Phase phase;
    phase.target = mapJsonToTransition(source["target"].as<JsonObject>());
    phase.stopConditions = mapJsonToPhaseStopConditions(source["stopConditions"].as<JsonObject>());
    phase.name = source["name"] ? std::string(source["name"]) : "";
    phase.type = mapJsonValueToPhaseType(source["type"]);
    phase.restriction = source["restriction"];
    phase.skip = source["skip"];
    phase.waterTemperature = source["waterTemperature"];
    return phase;
  }

  GlobalStopConditions mapJsonToGlobalStopConditions(const JsonObject& source) {
    GlobalStopConditions globalStopConditions;
    globalStopConditions.time = source["time"];
    globalStopConditions.weight = source["weight"];
    globalStopConditions.waterPumped = source["waterPumped"];
    return globalStopConditions;
  }

  Profile mapJsonToProfile(const JsonObject& source) {
    Profile profile;
    profile.name = source["name"] ? std::string(source["name"]) : "";
    JsonArray phasesJson = source["phases"].as<JsonArray>();
    for (JsonObject phaseJson : phasesJson) {
      profile.phases.push_back(mapJsonToPhase(phaseJson));
    }
    profile.globalStopConditions = mapJsonToGlobalStopConditions(source["globalStopConditions"].as<JsonObject>());
    profile.waterTemperature = source["waterTemperature"];
    profile.recipe = mapJsonToRecipe(source["recipe"].as<JsonObject>());
    return profile;
  }
}
