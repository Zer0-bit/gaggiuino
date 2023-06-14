#include "profiling_phases.h"
#include "proto/proto_serializer.h"
#include "proto/profile_converters.h"
#include "utils/test-utils.h"
#include <iostream>

void test_profile_serializer_works_correctly(void) {
  Profile profile{ .name = "test profile" };
  profile.phases.push_back({ .name = "Fill", .type = PhaseType::PRESSURE, .target = Transition(), .restriction = -1,  .stopConditions = PhaseStopConditions{} });
  profile.phases.push_back({ .type = PhaseType::PRESSURE, .target = Transition(10.f, TransitionCurve::EASE_IN_OUT, 1000), .restriction = -1,  .stopConditions = PhaseStopConditions{.time = 1000} });
  profile.phases.push_back({ .type = PhaseType::FLOW, .target = Transition(10.f, 5.f, TransitionCurve::EASE_IN), .restriction = 2.f,  .stopConditions = PhaseStopConditions{.weight = 10.f}, .waterTemperature = 95.f });
  profile.phases.push_back({ .type = PhaseType::FLOW, .target = Transition(0.f, 5.f, TransitionCurve::EASE_OUT), .restriction = 3.f,  .stopConditions = PhaseStopConditions{.pressureAbove = 2.f} });
  profile.phases.push_back({ .type = PhaseType::FLOW, .target = Transition(10.f, 5.f, TransitionCurve::LINEAR), .restriction = 3.f,  .stopConditions = PhaseStopConditions{.pressureAbove = 2.f, .flowAbove = 1.f, .flowBelow = 2.4f, .pressureBelow = 1.3f, .time = 199, .waterPumpedInPhase = 29, .weight = 20.f} });
  profile.globalStopConditions.time = 60000;
  profile.globalStopConditions.weight = 35.4f;
  profile.globalStopConditions.waterPumped = 120.3f;
  profile.waterTemperature = 93.f;

  std::vector<uint8_t> serializedProfile = ProtoSerializer::serialize<ProfileConverter>(profile);
  std::cout << "Output size: " << serializedProfile.size() << "(bytes)" << std::endl;

  Profile deserializedProfile;
  ProtoSerializer::deserialize<ProfileConverter>(serializedProfile, deserializedProfile);

  TEST_ASSERT_EQUAL_PROFILE(profile, deserializedProfile);
}


void test_profile_serializer_with_empty_profile(void) {
  Profile profile{ .name = "empty profile" };
  std::vector<uint8_t> serializedProfile = ProtoSerializer::serialize<ProfileConverter>(profile);
  std::cout << "Output size: " << serializedProfile.size() << "(bytes)" << std::endl;

  Profile deserializedProfile;
  ProtoSerializer::deserialize<ProfileConverter>(serializedProfile, deserializedProfile);

  TEST_ASSERT_EQUAL_PROFILE(profile, deserializedProfile);
}

void runAllProfileSerializerTests(void) {
  RUN_TEST(test_profile_serializer_works_correctly);
  RUN_TEST(test_profile_serializer_with_empty_profile);
}
