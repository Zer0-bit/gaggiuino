#ifndef MCU_COMMS_H
/* 09:32 15/03/2023 - change triggering comment */
#define MCU_COMMS_H

#include <Arduino.h>
#include "profiling_phases.h"
#include "SerialTransfer.h"
#include <vector>

#ifdef ESP32
#include "esp_task_wdt.h"
#endif

const uint8_t MAX_DATA_PER_PACKET_DEFAULT = 58;
const uint16_t HEARTBEAT_TIME_DELTA_MSEC = 2000;

enum class McuCommsMessageType : uint8_t {
  MCUC_HEARTBEAT = 1,

  MCUC_DATA_SHOT_SNAPSHOT = 2,
  MCUC_DATA_PROFILE = 3,
  MCUC_DATA_SENSOR_STATE_SNAPSHOT = 4,
  MCUC_DATA_WEIGHT = 5,

  // Request specific data
  MCUC_REQ_ACTIVE_PROFILE = 6,
  MCUC_REQ_SETTINGS = 7,

  // Commands
  MCUC_CMD_SAVE_PROFILE = 8,
  MCUC_CMD_SAVE_SETTINGS = 9,

  MCUC_RESPONSE = 10,
};

enum class McuCommsResponseResult : uint8_t {
  MCUC_OK = 0,
  MCUC_ERROR = 1,
};

struct McuCommsResponse {
  McuCommsMessageType type;
  McuCommsResponseResult result;
};

class ProfileSerializer {
public:
  size_t neededBufferSize(Profile& profile) const;
  std::vector<uint8_t> serializeProfile(Profile& profile) const;
  void deserializeProfile(std::vector<uint8_t>& data, Profile& profile) const;
};

class McuComms {
private:
  using ShotSnapshotReceivedCallback = void (*)(ShotSnapshot&);
  using ProfileReceivedCallback = void (*)(Profile&);
  using SensorStateSnapshotReceivedCallback = void (*)(SensorStateSnapshot&);
  using WeightReceivedCallback = void (*)(float);
  using ResponseReceivedCallback = void (*)(McuCommsResponse&);

  uint32_t lastByteReceived = 0;
  uint32_t lastHeartbeatSent = 0;
  ProfileSerializer profileSerializer;
  SerialTransfer transfer;
  ShotSnapshotReceivedCallback shotSnapshotCallback = nullptr;
  ProfileReceivedCallback profileCallback = nullptr;
  SensorStateSnapshotReceivedCallback sensorStateSnapshotCallback = nullptr;
  WeightReceivedCallback weightReceivedCallback = nullptr;
  ResponseReceivedCallback responseReceivedCallback = nullptr;
  Stream* debugPort = nullptr;
  size_t packetSize;

  /**
  * Structure inside each packet. We are adding the current packet sequence number and last packet sequence
  * number which greatly help in serialisation/deserialisation of large payloads
  *
  *   0x02     0x01     0x00     0x00   ....
  * |      | |      | |      | |______|__________________________________________ 2nd packet byte
  * |      | |      | |______|___________________________________________________ 1st packet byte
  * |      | |______|____________________________________________________________ index of current packet
  * |______|_____________________________________________________________________ index of last packet
  */
  void sendMultiPacket(std::vector<uint8_t>& buffer, size_t dataSize, uint8_t packetID);
  std::vector<uint8_t> receiveMultiPacket();
  void log(const char* format, ...) const;
  void logBufferHex(std::vector<uint8_t>& buffer, size_t dataSize) const;
  void establishConnection(uint32_t timeout);
  void sendHeartbeat();

  void shotSnapshotReceived(ShotSnapshot& snapshot) const;
  void profileReceived(Profile& profile) const;
  void sensorStateSnapshotReceived(SensorStateSnapshot& snapshot) const;
  void weightReceived(float weight) const;
  void responseReceived(McuCommsResponse& response) const;

public:
  void begin(Stream& serial, uint32_t waitConnectionMillis = 0, size_t packetSize = MAX_DATA_PER_PACKET_DEFAULT);
  void setDebugPort(Stream* debugPort);
  void setShotSnapshotCallback(ShotSnapshotReceivedCallback callback);
  void setProfileReceivedCallback(ProfileReceivedCallback callback);
  void setSensorStateSnapshotCallback(SensorStateSnapshotReceivedCallback callback);
  void setWeightReceivedCallback(WeightReceivedCallback callback);
  void setResponseReceivedCallback(ResponseReceivedCallback callback);

  void sendShotData(const ShotSnapshot& snapshot);
  void sendProfile(Profile& profile);
  void sendSensorStateSnapshot(const SensorStateSnapshot& snapshot);
  void sendWeight(float weight);
  void sendResponse(McuCommsResponse response);

  bool isConnected();
  void readDataAndTick();
};


#endif
