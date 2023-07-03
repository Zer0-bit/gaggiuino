#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <unity.h>
#include "profiling_phases.h"
#include "gaggia_settings.h"
#include "./test-utils.cpp"

void TEST_ASSERT_EQUAL_FLOAT_ACCURACY(float expected, float actual, int digits);

void TEST_ASSERT_EQUAL_GLOBAL_STOP_CONDITIONS(const GlobalStopConditions& expected, const GlobalStopConditions& actual);
void TEST_ASSERT_EQUAL_TRANSITION(const Transition& expected, const Transition& actual);
void TEST_ASSERT_EQUAL_PHASE_STOP_CONDITIONS(const PhaseStopConditions& expected, const PhaseStopConditions& actual);
void TEST_ASSERT_EQUAL_PHASE(const Phase& expected, const Phase& actual);
void TEST_ASSERT_EQUAL_PROFILE(const Profile& expected, const Profile& actual);

void TEST_ASSERT_EQUAL_GAGGIA_SETTINGS(const GaggiaSettings& expected, const GaggiaSettings& actual);

void printProfile(const Profile& profile);

#endif
