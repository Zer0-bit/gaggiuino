#include <unity.h>
#include "profiling_phases.h"
#include "utils.h"


Phase pressurePhase(float start, float end, unsigned int time) {
  return Phase{PHASE_TYPE_PRESSURE, start, end, 0.f, 0.f, time};
}

Phase phaseArray[] = {
    pressurePhase(0, 2, 1000),
    pressurePhase(2, 2, 10000),
    pressurePhase(2, 9, 1000),
    pressurePhase(9, 9, 10000),
    pressurePhase(9, 6, 30000),
};

Phases phases = {5, phaseArray};


void test_current_phase_calculation(void)
{
    CurrentPhase currentPhase;

    currentPhase = phases.getCurrentPhase(0);
    TEST_ASSERT_EQUAL(0, currentPhase.phaseIndex);
    TEST_ASSERT_EQUAL(0, currentPhase.timeInPhase);

    currentPhase = phases.getCurrentPhase(550);
    TEST_ASSERT_EQUAL(0, currentPhase.phaseIndex);
    TEST_ASSERT_EQUAL(550, currentPhase.timeInPhase);

    currentPhase = phases.getCurrentPhase(1000);
    TEST_ASSERT_EQUAL(1, currentPhase.phaseIndex);
    TEST_ASSERT_EQUAL(0, currentPhase.timeInPhase);

    currentPhase = phases.getCurrentPhase(5000);
    TEST_ASSERT_EQUAL(1, currentPhase.phaseIndex);
    TEST_ASSERT_EQUAL(4000, currentPhase.timeInPhase);

    currentPhase = phases.getCurrentPhase(30000);
    TEST_ASSERT_EQUAL(4, currentPhase.phaseIndex);
    TEST_ASSERT_EQUAL(8000, currentPhase.timeInPhase);
}

void test_get_pressure_for_phase(void)
{
    Phase phase = pressurePhase(0, 2, 1000);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, phase.getTarget(0));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, phase.getTarget(500));
    TEST_ASSERT_EQUAL_FLOAT(1.5f, phase.getTarget(750));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, phase.getTarget(1000));
}


void test_get_pressure_for_phase_with_negative_change(void)
{
    Phase phase = pressurePhase(9, 6, 3000);

    TEST_ASSERT_EQUAL_FLOAT(9.0f, phase.getTarget(0));
    TEST_ASSERT_EQUAL_FLOAT(8.0f, phase.getTarget(1000));
    TEST_ASSERT_EQUAL_FLOAT(7.0f, phase.getTarget(2000));
    TEST_ASSERT_EQUAL_FLOAT(6.0f, phase.getTarget(3000));
}

void test_get_pressure_for_phase_with_time_larger_than_duration(void)
{
    Phase phase = pressurePhase(9, 6, 3000);

    TEST_ASSERT_EQUAL_FLOAT(6.0f, phase.getTarget(4000));
    TEST_ASSERT_EQUAL_FLOAT(6.0f, phase.getTarget(10000));
}

void test_phases_with_zero_duration_are_skipped(void) {
    Phase array[] = {
      pressurePhase(2, 2, 0),
      pressurePhase(2, 5, 0),
      pressurePhase(2,5,0),
      pressurePhase(5, 5, 1000)
    };
    Phases phases = Phases {4, array};

    TEST_ASSERT_EQUAL(3, phases.getCurrentPhase(0).phaseIndex);
}

void runAllPressureProfilerTests() {
    RUN_TEST(test_current_phase_calculation);
    RUN_TEST(test_get_pressure_for_phase);
    RUN_TEST(test_get_pressure_for_phase_with_negative_change);
    RUN_TEST(test_get_pressure_for_phase_with_time_larger_than_duration);
    RUN_TEST(test_phases_with_zero_duration_are_skipped);
}
