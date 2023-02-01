#include "profiling_phases.h"
#include "mcu_comms.h"
#include "utils/test-utils.h"

void test_profile_serializer_works_correctly(void) {
  Phase phaseArray[] = {

  };

  Profile profile;
  profile.phases.push_back({PHASE_TYPE::PHASE_TYPE_PRESSURE, Transition(0.f, 10.f, TransitionCurve::EASE_IN_OUT, 1000), -1,  PhaseStopConditions{.time = 1000} });
  profile.phases.push_back({PHASE_TYPE::PHASE_TYPE_FLOW, Transition(10.f, 5.f, TransitionCurve::LINEAR), 2.f,  PhaseStopConditions{.weight = 10.f} });
  profile.phases.push_back({PHASE_TYPE::PHASE_TYPE_FLOW, Transition(10.f, 5.f, TransitionCurve::LINEAR), 3.f,  PhaseStopConditions{.pressureAbove = 2.f} });
  ProfileSerializer serializer;

  vector<uint8_t> serializedProfile = serializer.serializeProfile(profile);

  Profile deserializedProfile;
  serializer.deserializeProfile(serializedProfile, deserializedProfile);

  TEST_ASSERT_EQUAL_PROFILE(profile, deserializedProfile);
}

void runAllProfileSerializerTests(void) {
  RUN_TEST(test_profile_serializer_works_correctly);
}
