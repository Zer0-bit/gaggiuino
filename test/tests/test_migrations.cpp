#include <FlashStorage_STM32.hpp>
#include <iostream>

#include "../src/eeprom_data/eeprom_migration.h"
#include "../src/eeprom_data/eeprom_migration.cpp"
#include "../src/eeprom_data/legacy/eeprom_data_v12.h"
#include "../src/eeprom_data/eeprom_data.cpp"

void test_all_migrator_functions_are_defined(void) {
  for (int i = EEPROM_DATA_EARLIEST_VERSION; i < EEPROM_DATA_VERSION; i++) {
    TEST_ASSERT_TRUE_MESSAGE(eepromValueMigrators[i] != nullptr, ("Missing eeprom value migrator for  v" + std::to_string(i)).c_str());
    TEST_ASSERT_TRUE_MESSAGE(eepromValueLoaders[i] != nullptr, ("Missing eeprom value loader for  v" + std::to_string(i)).c_str());
    TEST_ASSERT_TRUE_MESSAGE(eepromValueInstantiators[i] != nullptr, ("Missing eeprom value instantiator for  v" + std::to_string(i)).c_str());
  }
}

eepromMetadata_t_v12 getStartingVersionData();
void test_migration_works_from_earilest_supported_version(void) {
  // Start from version4
  eepromMetadata_t_v12 startingData = getStartingVersionData();
  cout<< std::to_string(sizeof(eepromMetadata_t_v12)) << "\n";
  EEPROMProvider::get().put(0, startingData);
  cout<< std::to_string(sizeof(eepromMetadata_t_v12)) << "\n";
  EEPROMProvider::get().get(0, startingData);
  cout<< std::to_string(sizeof(eepromMetadata_t_v12)) << "\n";

  eepromValues_t targetValues = { .values = getDefaultGaggiaSettings() };
  bool result = migrate(EEPROM_DATA_EARLIEST_VERSION, EEPROM_DATA_VERSION, &targetValues);

  TEST_ASSERT_TRUE_MESSAGE(result, "Migration failed");
  TEST_ASSERT_EQUAL_MESSAGE(startingData.values.hpwr, targetValues.values.boiler.hpwr, "hpwr");
  TEST_ASSERT_EQUAL_MESSAGE(startingData.values.offsetTemp, targetValues.values.boiler.offsetTemp, "offsetTemp");

  for (int i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL_MESSAGE(startingData.values.profiles[i].setpoint, targetValues.values.profiles.savedProfiles[i].waterTemperature, "setpoint");
    TEST_ASSERT_EQUAL_MESSAGE(startingData.values.profiles[i].shotStopOnCustomWeight, targetValues.values.profiles.savedProfiles[i].recipe.coffeeOut, "shotStopOnCustomWeight");
  }
}

void runAllMigrationsTests(void) {
  RUN_TEST(test_all_migrator_functions_are_defined);
  RUN_TEST(test_migration_works_from_earilest_supported_version);
}


const eepromValues_t_v12::profile_t defaultsProfileV12[5] = {
  {"IUIUIU Classic",/*pi*/true, true, 10, 2.f, 3.f, 20, 4.f, 3.f, 600, true, 4.f,/*sk*/true, 7, 30, 0.f, 0.f, 2.f, 0.f, 4.f, 5, 2,/*tp*/true, false, 7.5f, 6.f, 0, 3.f, 4, 0, 3.f, 0.f, 0.f, 0, 0.f, 0, 0, 0.f,/*pf*/true, true, 0.f, 0.f, 0, 0, 0.f, 2.5f, 1.f, 15, 0, 6.f,/*other*/ 93, true, 18.f, 0.f, 2},  // profile 0
  {"Londinium",/*pi*/true, true, 0, 0.f, 9.f, 10, 4.f, 0.f, 650, true, 0.f,/*sk*/true, 15, 10, 0.f, 0.f, 0.7f, 0.f, 0.f, 1, 2,/*tp*/true, false, 9.f, 9.f, 4, 3.f, 0, 0, 3.f, 0.f, 0.f, 0, 0.f, 0, 0, 0.f,/*pf*/true, false, 9.f, 3.f, 20, 0, 3.f, 0.f, 0.f, 0, 0, 0.f,/*other*/ 92, true, 20.f, 0.f, 2},  // profile 1
  {"Adaptive",/*pi*/true, true, 0, 0.f, 7.f, 20, 3.f, 0.f, 600, true, 0.f,/*sk*/true, 0, 6, 3.f, 0.f, 0.f, 0.f, 0.f, 0, 3,/*tp*/true, false, 0.f, 9.f, 0, 0.f, 6, 0, 0.f, 0.f, 2.f, 0, 9.f, 6, 0, 9.f,/*pf*/true, true, 0.f, 0.f, 30, 0, 3.f, 0.f, 2.5f, 30, 0, 9.f,/*other*/ 93, true, 18.f, 0.f, 2},  // profile 2
  {"Filter 2.1",/*pi*/true, true, 0, 0.f, 4.f, 15, 1.f, 0.f, 600, true, 0.f,/*sk*/true, 0, 90, 0.f, 0.2f, 0.f, 0.f, 45.f, 0, 0,/*tp*/true, true, 0.f, 0.f, 0, 0.f, 0, 0, 0.f, 0.2f, 3.f, 0, 0.f, 10, 0, 9.f,/*pf*/true, true, 0.f, 0.f, 0, 0, 0.f, 3.f, 3.f, 5, 0, 9.f,/*other*/ 89, true, 18.f, 0.f, 2},  // profile 3
  {"Blooming espresso",/*pi*/true, true, 0, 0.f, 4.f, 20, 7.f, 0.f, 650, true, 0.f,/*sk*/true, 0, 30, 0.f, 0.f, 0.6f, 0.f, 5.f, 5, 2,/*tp*/true, true, 0.f, 0.f, 0, 0.f, 0, 0, 0.f, 2.f, 2.f, 1, 9.f, 0, 4, 9.f,/*pf*/true, true, 0.f, 0.f, 0, 0, 0.f, 2.f, 2.f, 15, 0, 6.f,/*other*/ 93, true, 18.f, 0.f, 2}  // profile 4
};

eepromMetadata_t_v12 getStartingVersionData() {
  uint32_t now = 192387; // can't be bothered to mock millis()

  eepromMetadata_t_v12 startingMetadata;
  startingMetadata.version = 12;
  startingMetadata.timestamp = now;
  startingMetadata.versionTimestampXOR = now ^ 4;
  startingMetadata.values = eepromValues_t_v12{
    .steamSetPoint = 93,
    .offsetTemp = 7,
    .hpwr = 400,
    .mainDivider = 20,
    .brewDivider = 50,
    .activeProfile = 0,
    .powerLineFrequency = 50,
    .lcdSleep = 15,
    .warmupState = false,
    .homeOnShotFinish = true,
    .brewDeltaState = false,
    .basketPrefill = true,
    .scalesF1 = 12309,
    .scalesF2 = 18223,
    .pumpFlowAtZero = 23.5,
    .ledState = false,
    .ledR = 1,
    .ledG = 2,
    .ledB = 3,
  };

  for (int i = 0; i < 5; i++) {
    startingMetadata.values.profiles[i] = defaultsProfileV12[i];
  }

  return startingMetadata;
}
