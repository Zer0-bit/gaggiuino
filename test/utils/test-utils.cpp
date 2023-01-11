#include "test-utils.h"

void TEST_ASSERT_EQUAL_FLOAT_ACCURACY(float expected, float actual, int digits) {
  float mult = pow(10, digits);
  TEST_ASSERT_EQUAL_FLOAT(roundf(expected * mult) / mult, roundf(actual * mult) / mult);
}

void TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(GlobalStopConditions& expected, GlobalStopConditions& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.time, actual.time, "GlobalStopConditions.time");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.waterPumped, actual.waterPumped, "GlobalStopConditions.waterPumped");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.weight, actual.weight, "GlobalStopConditions.weight");
}

void TEST_ASSERT_EQUAL_TRANSITION(Transition& expected, Transition& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.time, actual.time, "Transition.time");
  TEST_ASSERT_EQUAL_MESSAGE(expected.curve, actual.curve, "Transition.curve");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.start, actual.start, "Transition.start");
  TEST_ASSERT_EQUAL_MESSAGE(expected.end, actual.end, "Transition.end");
}

void TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(PhaseStopConditions& expected, PhaseStopConditions& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.time, actual.time, "PhaseStopConditions.time");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.pressureAbove, actual.pressureAbove, "PhaseStopConditions.pressureAbove");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.pressureBelow, actual.pressureBelow, "PhaseStopConditions.pressureBelow");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.flowAbove, actual.flowAbove, "PhaseStopConditions.flowAbove");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.flowBelow, actual.flowBelow, "PhaseStopConditions.flowBelow");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.weight, actual.weight, "PhaseStopConditions.weight");
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.waterPumpedInPhase, actual.waterPumpedInPhase, "PhaseStopConditions.waterPumpedInPhase");
}

void TEST_ASSERT_EQUAL_PHASE(Phase& expected, Phase& actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.type, actual.type, "Phase.type");
  TEST_ASSERT_EQUAL_TRANSITION(expected.target, actual.target);
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected.restriction, actual.restriction, "Phase.restriction");
  TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(expected.stopConditions, actual.stopConditions);
}

void TEST_ASSERT_EQUAL_PROFILE(Profile& expected, Profile& actual) {
  TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(expected.globalStopConditions, actual.globalStopConditions);
  for (int i = 0; i < expected.phaseCount(); i++) {
    TEST_ASSERT_EQUAL_PHASE(expected.phases[i], actual.phases[i]);
  }
}
