#include "test-utils.h"
#include <iostream>

void TEST_ASSERT_EQUAL_FLOAT_ACCURACY(float expected, float actual, int digits) {
  float mult = pow(10, digits);
  TEST_ASSERT_EQUAL_FLOAT(roundf(expected * mult) / mult, roundf(actual * mult) / mult);
}

void TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(const GlobalStopConditions& expected, const GlobalStopConditions& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.time, actual.time, "GlobalStopConditions.time");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.waterPumped, actual.waterPumped, "GlobalStopConditions.waterPumped");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.weight, actual.weight, "GlobalStopConditions.weight");
}

void TEST_ASSERT_EQUAL_TRANSITION(const Transition& expected, const Transition& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.time, actual.time, "Transition.time");
  TEST_ASSERT_EQUAL_MESSAGE(expected.curve, actual.curve, "Transition.curve");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.start, actual.start, "Transition.start");
  TEST_ASSERT_EQUAL_MESSAGE(expected.end, actual.end, "Transition.end");
}

void TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(const PhaseStopConditions& expected, const PhaseStopConditions& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.time, actual.time, "PhaseStopConditions.time");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.pressureAbove, actual.pressureAbove, "PhaseStopConditions.pressureAbove");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.pressureBelow, actual.pressureBelow, "PhaseStopConditions.pressureBelow");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.flowAbove, actual.flowAbove, "PhaseStopConditions.flowAbove");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.flowBelow, actual.flowBelow, "PhaseStopConditions.flowBelow");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.weight, actual.weight, "PhaseStopConditions.weight");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.waterPumpedInPhase, actual.waterPumpedInPhase, "PhaseStopConditions.waterPumpedInPhase");
}

void TEST_ASSERT_EQUAL_PHASE(const Phase& expected, const Phase& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.skip, actual.skip, "Phase.skip");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(expected.name.c_str(), actual.name.c_str(), "Phase.name");
  if (!expected.skip) { // If phase is skipped the other values don't matter semantically
    TEST_ASSERT_EQUAL_MESSAGE(expected.type, actual.type, "Phase.type");
    TEST_ASSERT_EQUAL_TRANSITION(expected.target, actual.target);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.restriction, actual.restriction, "Phase.restriction");
    TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(expected.stopConditions, actual.stopConditions);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.waterTemperature, actual.waterTemperature, "Phase.waterTemperature");
  }
}

void TEST_ASSERT_EQUAL_PROFILE(const Profile& expected, const Profile& actual) {
  TEST_ASSERT_EQUAL_STRING_MESSAGE(expected.name.c_str(), actual.name.c_str(), "Profile.name");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.waterTemperature, actual.waterTemperature, "Profile.waterTemperature");
  TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(expected.globalStopConditions, actual.globalStopConditions);
  for (int i = 0; i < expected.phaseCount(); i++) {
    TEST_ASSERT_EQUAL_PHASE(expected.phases[i], actual.phases[i]);
  }
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.recipe.coffeeIn, actual.recipe.coffeeIn, "profile.recipe.coffeeIn");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.recipe.coffeeOut, actual.recipe.coffeeOut, "profile.recipe.coffeeOut");
}

void TEST_ASSERT_EQUAL_GAGGIA_SETTINGS(const GaggiaSettings& expected, const GaggiaSettings& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.boiler.steamSetPoint, actual.boiler.steamSetPoint, "boiler.steamSetPoint");
  TEST_ASSERT_EQUAL_MESSAGE(expected.boiler.offsetTemp, actual.boiler.offsetTemp, "boiler.offsetTemp");
  TEST_ASSERT_EQUAL_MESSAGE(expected.boiler.hpwr, actual.boiler.hpwr, "boiler.hpwr");
  TEST_ASSERT_EQUAL_MESSAGE(expected.boiler.mainDivider, actual.boiler.mainDivider, "boiler.mainDivider");
  TEST_ASSERT_EQUAL_MESSAGE(expected.boiler.brewDivider, actual.boiler.brewDivider, "boiler.brewDivider");

  TEST_ASSERT_EQUAL_MESSAGE(expected.system.powerLineFrequency, actual.system.powerLineFrequency, "system.powerLineFrequency");
  TEST_ASSERT_EQUAL_MESSAGE(expected.system.pumpFlowAtZero, actual.system.pumpFlowAtZero, "system.pumpFlowAtZero");
  TEST_ASSERT_EQUAL_MESSAGE(expected.system.scalesF1, actual.system.scalesF1, "system.scalesF1");
  TEST_ASSERT_EQUAL_MESSAGE(expected.system.scalesF2, actual.system.scalesF2, "system.scalesF2");
  TEST_ASSERT_EQUAL_MESSAGE(expected.system.warmupState, actual.system.warmupState, "system.warmupState");
  TEST_ASSERT_EQUAL_MESSAGE(expected.system.lcdSleep, actual.system.lcdSleep, "screen.lcdSleep");

  TEST_ASSERT_EQUAL_MESSAGE(expected.brew.homeOnShotFinish, actual.brew.homeOnShotFinish, "screen.homeOnShotFinish");
  TEST_ASSERT_EQUAL_MESSAGE(expected.brew.brewDeltaState, actual.brew.brewDeltaState, "brew.brewDeltaState");
  TEST_ASSERT_EQUAL_MESSAGE(expected.brew.basketPrefill, actual.brew.basketPrefill, "brew.basketPrefill");

  TEST_ASSERT_EQUAL_MESSAGE(expected.led.state, actual.led.state, "led.state");
  TEST_ASSERT_EQUAL_MESSAGE(expected.led.disco, actual.led.disco, "led.disco");
  TEST_ASSERT_EQUAL_MESSAGE(expected.led.color.R, actual.led.color.R, "led.color.R");
  TEST_ASSERT_EQUAL_MESSAGE(expected.led.color.G, actual.led.color.G, "led.color.G");
  TEST_ASSERT_EQUAL_MESSAGE(expected.led.color.B, actual.led.color.B, "led.color.B");
}

#include <iostream>
#include <string>
#include <vector>

std::string transitionCurveToString(TransitionCurve curve) {
  switch (curve) {
  case TransitionCurve::EASE_IN_OUT: return "EASE_IN_OUT";
  case TransitionCurve::EASE_IN: return "EASE_IN";
  case TransitionCurve::EASE_OUT: return "EASE_OUT";
  case TransitionCurve::LINEAR: return "LINEAR";
  case TransitionCurve::INSTANT: return "INSTANT";
  default: return "Unknown";
  }
}

void printProfile(const Profile& profile) {
  std::cout << "Profile: \n";
  std::cout << "  Name: " << profile.name << "\n";
  std::cout << "  Water Temperature: " << profile.waterTemperature << "\n";

  std::cout << "  Recipe: \n";
  std::cout << "    Coffee In: " << profile.recipe.coffeeIn << "\n";
  std::cout << "    Coffee Out: " << profile.recipe.coffeeOut << "\n";
  std::cout << "    Brewing Ratio: " << profile.recipe.ratio << "\n";

  std::cout << "  Global Stop Conditions: \n";
  std::cout << "    Time: " << profile.globalStopConditions.time << "\n";
  std::cout << "    Weight: " << profile.globalStopConditions.weight << "\n";
  std::cout << "    Water Pumped: " << profile.globalStopConditions.waterPumped << "\n";

  std::cout << "  Phases:\n";
  for (const auto& phase : profile.phases) {
    std::cout << "    Phase: " << phase.name << "\n";
    std::cout << "      Type: " << (phase.type == PhaseType::FLOW ? "FLOW" : "PRESSURE") << "\n";
    std::cout << "      Skip: " << (phase.skip ? "true" : "false") << "\n";
    std::cout << "      Target Start: " << phase.target.start << "\n";
    std::cout << "      Target End: " << phase.target.end << "\n";
    std::cout << "      Target Time: " << phase.target.time << "\n";
    std::cout << "      Transition Curve: " << transitionCurveToString(phase.target.curve) << "\n";
    std::cout << "      Restriction: " << phase.restriction << "\n";
    std::cout << "      Water Temperature: " << phase.waterTemperature << "\n";

    std::cout << "      Stop Conditions:\n";
    std::cout << "        Time: " << phase.stopConditions.time << "\n";
    std::cout << "        Pressure Above: " << phase.stopConditions.pressureAbove << "\n";
    std::cout << "        Pressure Below: " << phase.stopConditions.pressureBelow << "\n";
    std::cout << "        Flow Above: " << phase.stopConditions.flowAbove << "\n";
    std::cout << "        Flow Below: " << phase.stopConditions.flowBelow << "\n";
    std::cout << "        Weight: " << phase.stopConditions.weight << "\n";
    std::cout << "        Water Pumped In Phase: " << phase.stopConditions.waterPumpedInPhase << "\n";
  }
}
