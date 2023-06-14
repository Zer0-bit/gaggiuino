#ifndef MCU_COMMS_H
/* 09:32 15/03/2023 - change triggering comment */
#define MCU_COMMS_H

#include <Arduino.h>
#include "profiling_phases.h"
#include "SerialTransfer.h"
#include <vector>
#include <functional>

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
  MCUC_DATA_REMOTE_SCALES_WEIGHT = 5,
  MCUC_DATA_REMOTE_SCALES_DISCONNECTED = 6,

  // Request specific data
  MCUC_REQ_ACTIVE_PROFILE = 7,
  MCUC_REQ_SETTINGS = 8,

  // Commands
  MCUC_CMD_SAVE_PROFILE = 9,
  MCUC_CMD_SAVE_SETTINGS = 10,
  MCUC_CMD_REMOTE_SCALES_TARE = 11,

  MCUC_RESPONSE = 12,
};

enum class McuCommsResponseResult : uint8_t {
  MCUC_OK = 0,
  MCUC_ERROR = 1,
};

struct McuCommsResponse {
  McuCommsMessageType type;
  McuCommsResponseResult result;
};

class McuComms {
private:
  using ShotSnapshotReceivedCallback = std::function<void(ShotSnapshot&)>;
  using ProfileReceivedCallback = std::function<void(Profile&)>;
  using SensorStateSnapshotReceivedCallback = std::function<void(SensorStateSnapshot&)>;
  using ResponseReceivedCallback = std::function<void(McuCommsResponse&)>;
  using RemoteScalesWeightReceivedCallback = std::function<void(float)>;
  using RemoteScalesTareCommandCallback = std::function<void()>;
  using RemoteScalesDisconnectedCallback = std::function<void()>;

  uint32_t lastByteReceived = 0;
  uint32_t lastHeartbeatSent = 0;
  SerialTransfer transfer;
  ShotSnapshotReceivedCallback shotSnapshotCallback = nullptr;
  ProfileReceivedCallback profileCallback = nullptr;
  SensorStateSnapshotReceivedCallback sensorStateSnapshotCallback = nullptr;
  ResponseReceivedCallback responseReceivedCallback = nullptr;
  RemoteScalesWeightReceivedCallback remoteScalesWeightReceivedCallback = nullptr;
  RemoteScalesTareCommandCallback remoteScalesTareCommandCallback = nullptr;
  RemoteScalesDisconnectedCallback remoteScalesDisconnectedCallback = nullptr;
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
  void responseReceived(McuCommsResponse& response) const;
  void remoteScalesWeightReceived(float weight) const;
  void remoteScalesTareCommandReceived() const;
  void remoteScalesDisconnected() const;

public:
  void begin(Stream& serial, uint32_t waitConnectionMillis = 0, size_t packetSize = MAX_DATA_PER_PACKET_DEFAULT);
  void setDebugPort(Stream* debugPort);
  void setShotSnapshotCallback(ShotSnapshotReceivedCallback callback);
  void setProfileReceivedCallback(ProfileReceivedCallback callback);
  void setSensorStateSnapshotCallback(SensorStateSnapshotReceivedCallback callback);
  void setResponseReceivedCallback(ResponseReceivedCallback callback);
  void setRemoteScalesWeightReceivedCallback(RemoteScalesWeightReceivedCallback callback);
  void setRemoteScalesTareCommandCallback(RemoteScalesTareCommandCallback callback);
  void setRemoteScalesDisconnectedCallback(RemoteScalesDisconnectedCallback callback);

  void sendShotData(const ShotSnapshot& snapshot);
  void sendProfile(Profile& profile);
  void sendSensorStateSnapshot(const SensorStateSnapshot& snapshot);
  void sendResponse(McuCommsResponse response);
  void sendRemoteScalesWeight(float weight);
  void sendRemoteScalesTare();
  void sendRemoteScalesDisconnected();

  bool isConnected();
  void readDataAndTick();
};


#endif
