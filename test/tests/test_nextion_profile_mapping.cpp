#include <unity.h>
#include <profiling_phases.h>
#include "../src/lcd/nextion_profile_mapping.h"
#include "../src/lcd/nextion_profile_mapping.cpp"
#include "../src/eeprom_data/new_default_profiles.h"
#include "utils/test-utils.h"

void test_mapping_profile_to_and_from_nextion_works(void) {
  for (auto& profile : defaultProfiles) {
    std::cout << "Input Profile: " << profile.name << std::endl;
    // printProfile(profile); // Enable for debugging and run VerboseTest


    nextion_profile_t nextionProfile;
    mapProfileToNextionProfile(profile, nextionProfile);
    Profile reversedProfile;
    mapNextionProfileToProfile(nextionProfile, reversedProfile);

    std::cout << "Success: " << profile.name << std::endl;
    // printProfile(reversedProfile);  // Enable for debugging and run VerboseTest

    TEST_ASSERT_EQUAL_PROFILE(profile, reversedProfile);
  }
}

void runAllNextionProfileMappingTests(void) {
  RUN_TEST(test_mapping_profile_to_and_from_nextion_works);
}

