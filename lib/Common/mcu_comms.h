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
  MCUC_DATA_ALL_SETTINGS = 7,
  MCUC_DATA_BREW_SETTINGS = 8,
  MCUC_DATA_BOILER_SETTINGS = 9,
  MCUC_DATA_SYSTEM_SETTINGS = 10,
  MCUC_DATA_LED_SETTINGS = 11,
  MCUC_DATA_NOTIFICATION = 12,
  MCUC_DATA_MANUAL_BREW_PHASE = 13,
  MCUC_DATA_SYSTEM_STATE = 14,

  // Request specific data
  MCUC_REQ_DATA = 15,

  // Commands
  MCUC_CMD_REMOTE_SCALES_TARE = 16,
};

struct McuCommsRequestData {
  McuCommsMessageType type;
};

class McuComms {
private:
  using MessageReceivedCallback = std::function<void(McuCommsMessageType, std::vector<uint8_t>&)>;

  uint32_t lastByteReceived = 0;
  uint32_t lastHeartbeatSent = 0;
  SerialTransfer transfer;
  MessageReceivedCallback messageReceivedCallback = nullptr;
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
  void sendMultiPacket(const std::vector<uint8_t>& buffer, McuCommsMessageType messageType);
  std::vector<uint8_t> receiveMultiPacket();
  void log(const char* format, ...) const;
  void logBufferHex(const std::vector<uint8_t>& buffer, size_t dataSize) const;
  void establishConnection(uint32_t timeout);
  void sendHeartbeat();

  void shotSnapshotReceived(ShotSnapshot& snapshot) const;
  void profileReceived(Profile& profile) const;
  void sensorStateSnapshotReceived(SensorStateSnapshot& snapshot) const;
  void remoteScalesWeightReceived(float weight) const;
  void remoteScalesTareCommandReceived() const;
  void remoteScalesDisconnected() const;

public:
  void begin(Stream& serial, uint32_t waitConnectionMillis = 0, size_t packetSize = MAX_DATA_PER_PACKET_DEFAULT);
  void setDebugPort(Stream* debugPort);
  void setMessageReceivedCallback(MessageReceivedCallback callback);

  // Sends a message with no data. Useful for commands. This is equivalent to sending the message with an empty data vector
  void sendMessage(McuCommsMessageType messageType);
  // Sends a message of a given type with its corresponding data. Sender and receiver must agree on how this data is serialized/deserialized
  void sendMessage(McuCommsMessageType messageType, const std::vector<uint8_t>& data);

  bool isConnected();
  void readDataAndTick();
};

#endif
