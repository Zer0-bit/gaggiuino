#include "test-utils.h"

void TEST_ASSERT_EQUAL_FLOAT_ACCURACY(float expected, float actual, int digits) {
  float mult = pow(10, digits);
  TEST_ASSERT_EQUAL_FLOAT(roundf(expected * mult) / mult, roundf(actual * mult) / mult);
}

void TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(GlobalStopConditions& expected, GlobalStopConditions& actual) {
  TEST_ASSERT_EQUAL(expected.time, actual.time);
  TEST_ASSERT_EQUAL_FLOAT(expected.waterPumped, actual.waterPumped);
  TEST_ASSERT_EQUAL_FLOAT(expected.weight, actual.weight);
}

void TEST_ASSERT_EQUAL_TRANSITION(Transition& expected, Transition& actual) {
  TEST_ASSERT_EQUAL(expected.time, actual.time);
  TEST_ASSERT_EQUAL(expected.curve, actual.curve);
  TEST_ASSERT_EQUAL_FLOAT(expected.start, actual.start);
  TEST_ASSERT_EQUAL(expected.end, actual.end);
}

void TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(PhaseStopConditions& expected, PhaseStopConditions& actual) {
  TEST_ASSERT_EQUAL(expected.time, actual.time);
  TEST_ASSERT_EQUAL_FLOAT(expected.pressureAbove, actual.pressureAbove);
  TEST_ASSERT_EQUAL_FLOAT(expected.pressureBelow, actual.pressureBelow);
  TEST_ASSERT_EQUAL_FLOAT(expected.flowAbove, actual.flowAbove);
  TEST_ASSERT_EQUAL_FLOAT(expected.flowBelow, actual.flowBelow);
  TEST_ASSERT_EQUAL_FLOAT(expected.weight, actual.weight);
  TEST_ASSERT_EQUAL_FLOAT(expected.waterPumpedInPhase, actual.waterPumpedInPhase);
}

void TEST_ASSERT_EQUAL_PHASE(Phase& expected, Phase& actual) {
  TEST_ASSERT_EQUAL(expected.type, actual.type);
  TEST_ASSERT_EQUAL_TRANSITION(expected.target, actual.target);
  TEST_ASSERT_EQUAL_FLOAT(expected.restriction, actual.restriction);
  TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(expected.stopConditions, actual.stopConditions);
}

void TEST_ASSERT_EQUAL_PROFILE(Profile& expected, Profile& actual) {
  TEST_ASSERT_EQUAL(expected.count, actual.count);
  TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(expected.globalStopConditions, actual.globalStopConditions);
  for (int i = 0; i < expected.count; i++) {
    TEST_ASSERT_EQUAL_PHASE(expected.phases[i], actual.phases[i]);
  }
}
