#ifndef JSON_PROFILE_CONVERTERS_H
#define JSON_PROFILE_CONVERTERS_H

#include "ArduinoJson.h"
#include "profiling_phases.h"
#include "../../persistence/saved_profiles.h"

namespace json {

  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  std::string mapTransitionCurveToJsonValue(const TransitionCurve curve);
  std::string mapPhaseTypeToJsonValue(const PhaseType phaseType);
  void mapRecipeToJson(const BrewRecipe& recipe, JsonObject& target);
  void mapTransitionToJson(const Transition& transition, JsonObject& target);
  void mapPhaseStopConditionsToJson(const PhaseStopConditions& stopConditions, JsonObject& target);
  void mapPhaseToJson(const Phase& phase, JsonObject& target);
  void mapGlobalStopConditionsToJson(const GlobalStopConditions& globalStopConditions, JsonObject& target);
  void mapProfileToJson(ProfileId id, const Profile& profile, JsonObject& target);
  void mapProfileSummaryToJson(const SavedProfile& summary, JsonObject& target);

  // ------------------------------------------------------------------------------------------
  // ----------------------------------- Deserializers ----------------------------------------
  // ------------------------------------------------------------------------------------------

  TransitionCurve mapJsonValueToTransitionCurve(const std::string& curve);
  PhaseType mapJsonValueToPhaseType(const std::string& phaseType);
  BrewRecipe mapJsonToRecipe(const JsonObject& source);
  Transition mapJsonToTransition(const JsonObject& source);
  PhaseStopConditions mapJsonToPhaseStopConditions(const JsonObject& source);
  Phase mapJsonToPhase(const JsonObject& source);
  GlobalStopConditions mapJsonToGlobalStopConditions(const JsonObject& source);
  Profile mapJsonToProfile(const JsonObject& source);

}
#endif
