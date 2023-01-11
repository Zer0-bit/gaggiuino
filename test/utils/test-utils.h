#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <unity.h>
#include "profiling_phases.h"
#include "./test-utils.cpp"

void TEST_ASSERT_EQUAL_FLOAT_ACCURACY(float expected, float actual, int digits);

void TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(GlobalStopConditions& expected, GlobalStopConditions& actual);
void TEST_ASSERT_EQUAL_TRANSITION(Transition& expected, Transition& actual);
void TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(PhaseStopConditions& expected, PhaseStopConditions& actual);
void TEST_ASSERT_EQUAL_PHASE(Phase& expected, Phase& actual);
void TEST_ASSERT_EQUAL_PROFILE(Profile& expected, Profile& actual);

#endif
