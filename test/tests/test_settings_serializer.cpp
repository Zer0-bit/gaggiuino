#include "gaggia_settings.h"
#include "proto/proto_serializer.h"
#include "proto/settings_converters.h"
#include "../webserver/src/persistence/default_settings.h"
#include "utils/test-utils.h"
#include "mcu_comms.h"
#include "proto/message_converters.h"
#include <iostream>

void test_settings_serializer_works_correctly(void) {
  GaggiaSettings settings = default_settings::getDefaultSettings();

  std::vector<uint8_t> serializedSettings = ProtoSerializer::serialize<GaggiaSettingsConverter>(settings);
  std::cout << "Output size: " << serializedSettings.size() << "(bytes)" << std::endl;

  GaggiaSettings deserializedSettings;
  ProtoSerializer::deserialize<GaggiaSettingsConverter>(serializedSettings, deserializedSettings);

  TEST_ASSERT_EQUAL_GAGGIA_SETTINGS(settings, deserializedSettings);
}

void test_data_request_is_serialized_correctly(void) {
  McuCommsRequestData requestData = { .type = McuCommsMessageType::MCUC_DATA_ALL_SETTINGS };

  std::vector<uint8_t> serializedRequest = ProtoSerializer::serialize<McuCommsRequestDataConverter>(requestData);
  std::cout << "Output size: " << serializedRequest.size() << "(bytes)" << std::endl;

  McuCommsRequestData deserializedRequest;
  ProtoSerializer::deserialize<McuCommsRequestDataConverter>(serializedRequest, deserializedRequest);

  TEST_ASSERT_EQUAL_MESSAGE(requestData.type, deserializedRequest.type, "not equal");
}

void runAllSettingsSerializerTests(void) {
  RUN_TEST(test_settings_serializer_works_correctly);
  RUN_TEST(test_data_request_is_serialized_correctly);
}
