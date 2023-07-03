#include "gaggia_settings.h"
#include "proto/proto_serializer.h"
#include "proto/settings_converters.h"
#include "../webserver/src/persistence/default_settings.h"
#include "utils/test-utils.h"
#include <iostream>

void test_settings_serializer_works_correctly(void) {
  GaggiaSettings settings = default_settings::getDefaultSettings();

  std::vector<uint8_t> serializedSettings = ProtoSerializer::serialize<GaggiaSettingsConverter>(settings);
  std::cout << "Output size: " << serializedSettings.size() << "(bytes)" << std::endl;

  GaggiaSettings deserializedSettings;
  ProtoSerializer::deserialize<GaggiaSettingsConverter>(serializedSettings, deserializedSettings);

  TEST_ASSERT_EQUAL_GAGGIA_SETTINGS(settings, deserializedSettings);
}

void runAllSettingsSerializerTests(void) {
  RUN_TEST(test_settings_serializer_works_correctly);
}
