#include <unity.h>
#include <profiling_phases.h>
#include <sensors_state.h>
#include <utils.h>

Phase pressurePhase(float start, float end, long time) {
  Phase phase = Phase{PHASE_TYPE::PHASE_TYPE_PRESSURE, Transition{start, end}, 0.f, PhaseStopConditions{}};
  phase.stopConditions.time = time;
  return phase;
}

Phase presurePhaseWithWeightTarget(float start, float end, long time, float weight) {
  Phase phase = Phase{PHASE_TYPE::PHASE_TYPE_PRESSURE, Transition{start, end}, 0.f, PhaseStopConditions{}};
  phase.stopConditions.time = time;
  phase.stopConditions.weight = weight;
  return phase;
}

SensorState state;

void test_current_phase_calculation(void)
{
    Profile profile;
    profile.phases.push_back(pressurePhase(0, 2, 1000));
    profile.phases.push_back(pressurePhase(2, 2, 10000));
    profile.phases.push_back(pressurePhase(2, 9, 1000));
    profile.phases.push_back(pressurePhase(9, 9, 10000));
    profile.phases.push_back(pressurePhase(9, 6, 30000));

    PhaseProfiler phaseProfiler = PhaseProfiler(profile);
    phaseProfiler.reset();
    phaseProfiler.updatePhase(0, state);
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(550, state);
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(550, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(1000, state);
    TEST_ASSERT_EQUAL(1, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(5000, state);
    TEST_ASSERT_EQUAL(1, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(4000, phaseProfiler.getCurrentPhase().getTimeInPhase());

    // phase switch triggered on 11.5sec
    phaseProfiler.updatePhase(11500, state);
    TEST_ASSERT_EQUAL(2, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getTimeInPhase());

    // still in the same phase as it should last 1000msec
    phaseProfiler.updatePhase(12499, state);
    TEST_ASSERT_EQUAL(2, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(999, phaseProfiler.getCurrentPhase().getTimeInPhase());

    // still in the same phase as it should last 1000msec
    phaseProfiler.updatePhase(12500, state);
    TEST_ASSERT_EQUAL(3, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getTimeInPhase());
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
    Profile profile;
    profile.addPhase(pressurePhase(2, 2, 0));
    profile.addPhase(pressurePhase(2, 5, 0));
    profile.addPhase(pressurePhase(2, 5, 0));
    profile.addPhase(pressurePhase(5, 5, 1000));

    PhaseProfiler profiler = PhaseProfiler(profile);

    profiler.updatePhase(0, state);
    TEST_ASSERT_EQUAL(3, profiler.getCurrentPhase().getIndex());
}


void test_phases_with_weight_stop_condition(void) {
  SensorState mockedState;
  mockedState.shotWeight = 0.f;
  mockedState.smoothedPumpFlow = 0.f;
  mockedState.weightFlow = 0.f;

  float weightTarget = 0.4f;
  Profile profile;
  profile.addPhase(presurePhaseWithWeightTarget(2, 2, -1, weightTarget));
  profile.addPhase(pressurePhase(5, 5, 1000));

  PhaseProfiler profiler = PhaseProfiler(profile);

  mockedState.shotWeight = 0.2f;
  profiler.updatePhase(3000, mockedState);
  TEST_ASSERT_FALSE(profiler.isFinished());

  mockedState.shotWeight = 0.5f;
  profiler.updatePhase(3000, mockedState);
  TEST_ASSERT_FALSE(profiler.isFinished());

  profiler.updatePhase(3500, mockedState);
  TEST_ASSERT_EQUAL(1, profiler.getCurrentPhase().getIndex());
  TEST_ASSERT_EQUAL(500, profiler.getCurrentPhase().getTimeInPhase());
  TEST_ASSERT_FALSE(profiler.isFinished());

  profiler.updatePhase(4100, mockedState);
  TEST_ASSERT_EQUAL(0, profiler.getCurrentPhase().getTimeInPhase());
  TEST_ASSERT_TRUE(profiler.isFinished());
}

void test_phases_with_stop_conditions_and_skipped_phases() {
  SensorState mockedState;
  mockedState.shotWeight = 0.f;
  mockedState.smoothedPumpFlow = 0.f;
  mockedState.weightFlow = 0.f;

  float weightTarget = 0.4f;

  Profile profile;
  profile.addPhase(presurePhaseWithWeightTarget(2, 2, 30000, weightTarget));
  profile.addPhase(pressurePhase(0, 0, 0)); // should be skipped;
  profile.addPhase(pressurePhase(5, 5, 1000));

  PhaseProfiler profiler = PhaseProfiler{profile};

  mockedState.shotWeight = 0.5f;
  profiler.updatePhase(2000, mockedState);
  TEST_ASSERT_FALSE(profiler.isFinished());
  TEST_ASSERT_EQUAL(2, profiler.getCurrentPhase().getIndex());
}

void test_phases_stay_constant() {
  // Check that the phases stay constant after updating and resetting
  Profile profile;
  profile.phases.push_back(pressurePhase(0, 2, 1000));
  profile.phases.push_back(pressurePhase(2, 2, 10000));
  profile.phases.push_back(pressurePhase(2, 9, 1000));
  profile.phases.push_back(pressurePhase(9, 9, 10000));
  profile.phases.push_back(pressurePhase(9, 6, 30000));
  PhaseProfiler profiler(profile);

  profiler.reset();
  profiler.updatePhase(0, state);
  profiler.updatePhase(550, state);
  profiler.updatePhase(1000, state);
  profiler.updatePhase(12500, state);
  profiler.reset();

  TEST_ASSERT_EQUAL(0, profiler.getCurrentPhase().getIndex());
  TEST_ASSERT_EQUAL(0, profiler.getCurrentPhase().getTimeInPhase());

  TEST_ASSERT_EQUAL(1000, profile.phases[0].stopConditions.time);
  TEST_ASSERT_EQUAL(0, profile.phases[0].target.start);
  TEST_ASSERT_EQUAL(2, profile.phases[0].target.end);

  TEST_ASSERT_EQUAL(10000, profile.phases[1].stopConditions.time);
  TEST_ASSERT_EQUAL(2, profile.phases[1].target.start);
  TEST_ASSERT_EQUAL(2, profile.phases[1].target.end);
}

void runAllPressureProfilerTests() {
  RUN_TEST(test_current_phase_calculation);
  RUN_TEST(test_get_pressure_for_phase);
  RUN_TEST(test_get_pressure_for_phase_with_negative_change);
  RUN_TEST(test_get_pressure_for_phase_with_time_larger_than_duration);
  RUN_TEST(test_phases_with_zero_duration_are_skipped);
  RUN_TEST(test_phases_with_weight_stop_condition);
  RUN_TEST(test_phases_with_stop_conditions_and_skipped_phases);
  RUN_TEST(test_phases_stay_constant);
}
