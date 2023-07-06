#include "gaggia_settings.h"
#include "proto/proto_serializer.h"
#include "proto/settings_converters.h"
#include "proto/message_converters.h"
#include "../webserver/src/persistence/default_settings.h"
#include "utils/test-utils.h"
#include "mcu_comms.h"
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

void test_shot_snapshot_is_serialized_correctly(void) {
  ShotSnapshot snapshot = {
    .timeInShot = 13,
    .pressure = 1.f,
    .pumpFlow = 2.f,
    .weightFlow = 3.f,
    .temperature = 98.f,
    .shotWeight = 3.f,
    .waterPumped = 4.f,

    .targetTemperature = 100.f,
    .targetPumpFlow = 12.f,
    .targetPressure = 5.f,
  };

  std::vector<uint8_t> serializedData = ProtoSerializer::serialize<ShotSnapshotConverter>(snapshot);
  std::cout << "Output size: " << serializedData.size() << "(bytes)" << std::endl;

  ShotSnapshot deserializedSnapshot;
  ProtoSerializer::deserialize<ShotSnapshotConverter>(serializedData, deserializedSnapshot);

  TEST_ASSERT_EQUAL_SHOT_SNAPSHOT(snapshot, deserializedSnapshot);
}

void test_system_state_serialized_correctly(void) {
  SystemState systemState = {
    .startupInitFinished = true,
    .operationMode = OperationMode::FLUSH,
    .tofReady = true,
    .isSteamForgottenON = true,
    .scalesPresent = true,
    .timeAlive = 123123,
    .descaleProgress = 12,
  };

  std::vector<uint8_t> serializedData = ProtoSerializer::serialize<SystemStateConverter>(systemState);
  std::cout << "Output size: " << serializedData.size() << "(bytes)" << std::endl;

  SystemState deserializedSystemState;
  ProtoSerializer::deserialize<SystemStateConverter>(serializedData, deserializedSystemState);

  TEST_ASSERT_EQUAL_SYSTEM_STATE(systemState, deserializedSystemState);
}

void test_measurement_serialized_correctly(void) {
  Measurement weight = {
    .value = 10.3f,
  };

  std::vector<uint8_t> serializedData = ProtoSerializer::serialize<MeasurementConverter>(weight);
  std::cout << "Output size: " << serializedData.size() << "(bytes)" << std::endl;

  Measurement deserializedWeight = {};
  ProtoSerializer::deserialize<MeasurementConverter>(serializedData, deserializedWeight);

  TEST_ASSERT_EQUAL_MESSAGE(weight.value, deserializedWeight.value, "value");
  TEST_ASSERT_EQUAL_MESSAGE(weight.millis, deserializedWeight.millis, "millis");
}

void test_notification_serialized_correctly(void) {
  Notification notification = Notification::info("some notification");

  std::vector<uint8_t> serializedData = ProtoSerializer::serialize<NotificationConverter>(notification);
  std::cout << "Output size: " << serializedData.size() << "(bytes)" << std::endl;

  Notification deserializedNotification = {};
  ProtoSerializer::deserialize<NotificationConverter>(serializedData, deserializedNotification);

  TEST_ASSERT_EQUAL_MESSAGE(notification.type, deserializedNotification.type, "type");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(notification.message.c_str(), deserializedNotification.message.c_str(), "message");
}

void runAllSettingsSerializerTests(void) {
  RUN_TEST(test_settings_serializer_works_correctly);
  RUN_TEST(test_data_request_is_serialized_correctly);
  RUN_TEST(test_shot_snapshot_is_serialized_correctly);
  RUN_TEST(test_system_state_serialized_correctly);
  RUN_TEST(test_measurement_serialized_correctly);
  RUN_TEST(test_notification_serialized_correctly);
}
