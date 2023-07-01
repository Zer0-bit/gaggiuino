/* 09:32 15/03/2023 - change triggering comment */
#include "mcu_comms.h"
#include "proto/proto_serializer.h"
#include "proto/profile_converters.h"
#include "proto/message_converters.h"
#include <stdarg.h>

using namespace std;

//---------------------------------------------------------------------------------
//---------------------------    PRIVATE METHODS       ----------------------------
//---------------------------------------------------------------------------------
// This is here to allow sending empty messages without allocating/deallocating memory for an empty vector
const std::vector<uint8_t>& emptyVector() {
  static const std::vector<uint8_t> emptyVec;
  return emptyVec;
}

// Sends a message in multiple packets if necessary
void McuComms::sendMultiPacket(const vector<uint8_t>& buffer, McuCommsMessageType messageType) {
  uint8_t packetID = static_cast<uint8_t>(messageType);
  size_t dataSize = buffer.size();
  log("Sending buffer[%d]: ", dataSize);
  logBufferHex(buffer, dataSize);

  auto dataPerPacket = static_cast<uint8_t>(packetSize - 2u); // Two bytes are reserved for current index and last index
  auto numPackets = static_cast<uint8_t>(dataSize / dataPerPacket);

  if (numPackets == 0 || dataSize % dataPerPacket > 0u) // Add an extra transmission if needed
    numPackets++;

  for (uint8_t currentPacket = 0u; currentPacket < numPackets; currentPacket++) {
    uint8_t dataLen = dataPerPacket;


    if (((currentPacket + 1u) * dataPerPacket) > dataSize) // Determine data length for the last packet if file length is not an exact multiple of `dataPerPacket`
      dataLen = static_cast<uint8_t>(dataSize - currentPacket * dataPerPacket);

    uint16_t sendSize = transfer.txObj(numPackets - 1u, 0u); // index of last packet
    sendSize = transfer.txObj(currentPacket, (uint16_t)1); // index of current packet
    sendSize = transfer.txObj(buffer[currentPacket * dataPerPacket], (uint16_t)2, dataLen); // packet payload

    transfer.sendData(sendSize, packetID); // Send the current file index and data
  }
  log("Data sent.\n");
}

vector<uint8_t> McuComms::receiveMultiPacket() {
  uint8_t lastPacket = transfer.packet.rxBuff[0]; // Get index of last packet
  uint8_t currentPacket = transfer.packet.rxBuff[1]; // Get index of current packet
  uint8_t bytesRead = transfer.bytesRead; // Bytes read in current packet
  uint8_t dataPerPacket = bytesRead - (uint8_t)2; // First 2 bytes of each packet are used as indexes and are not put in the buffer
  size_t  totalBytes = 0u;

  vector<uint8_t> buffer;
  buffer.reserve((lastPacket + 1u) * dataPerPacket);

  while (currentPacket <= lastPacket) {
#ifdef ESP32
    esp_task_wdt_reset();
#endif

    log("Handling packet %d\n", currentPacket);
    totalBytes += bytesRead - 2u;

    // First 2 bytes are not added to the buffer because they represent the index of current and last packet
    for (uint8_t i = 0u; i < bytesRead - 2u; i++) {
      buffer.push_back(transfer.packet.rxBuff[i + 2u]);
    }
    if (currentPacket == lastPacket) break;

    // wait for more data to become available for up to 20 milliseconds
    uint8_t dataAvailable = transfer.available();

    uint32_t waitStartedAt = millis();
    while (millis() - waitStartedAt < 50u && !dataAvailable) {
#ifdef ESP32
      esp_task_wdt_reset();
#endif
      dataAvailable = transfer.available();
    }

    // if data is not available exit the loop
    if (!dataAvailable) {
      log("ERROR: esp_stm_comms read timeout error\n");
      break;
    }

    // if data is available parse the current packet and number of packets and put data in the buffer
    lastPacket = transfer.packet.rxBuff[0];
    currentPacket = transfer.packet.rxBuff[1];
    bytesRead = transfer.bytesRead;
  }

  log("Received buffer[%d]: ", totalBytes);
  logBufferHex(buffer, totalBytes);

  return buffer;
}

void McuComms::log(const char* format, ...) const {
  if (!debugPort) return;

  std::array<char, 128>buffer;
  va_list args;
  va_start(args, format);
  vsnprintf(buffer.data(), buffer.size(), format, args);
  va_end(args);
  debugPort->print("McuComms: ");
  debugPort->print(buffer.data());
}

void McuComms::logBufferHex(const vector<uint8_t>& buffer, size_t dataSize) const {
  if (!debugPort) return;

  std::array<char, 3>hex;
  for (size_t i = 0u; i < dataSize; i++) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(hex.data(), hex.max_size(), "%02x ", buffer[i]);
#pragma GCC diagnostic pop
    debugPort->print(hex.data());
  }
  debugPort->println();
}

void McuComms::establishConnection(uint32_t timeout) {
  if (timeout <= 0) return;

  uint32_t waitingStart = millis();

  while (millis() - waitingStart < timeout && lastByteReceived == 0) {
    sendHeartbeat();
    readDataAndTick();
    delay(10);
  }

  if (lastByteReceived > 0) {
    log("Successful connection after=%dms", millis() - waitingStart);
  }
  else {
    log("Unsuccessful connection after=%dms", millis() - waitingStart);
  }
}

void McuComms::sendHeartbeat() {
  uint16_t messageSize = transfer.txObj(static_cast<uint8_t>(McuCommsMessageType::MCUC_HEARTBEAT));
  transfer.sendData(messageSize, static_cast<uint8_t>(McuCommsMessageType::MCUC_HEARTBEAT));
}

//---------------------------------------------------------------------------------
//---------------------------    PUBLIC METHODS       ----------------------------
//---------------------------------------------------------------------------------
void McuComms::begin(Stream& serial, uint32_t waitConnectionMillis, size_t packetSize) {
  McuComms::packetSize = packetSize;
  log("Staring with packetSize: %d\n", packetSize);
  transfer.begin(serial, true);
  establishConnection(waitConnectionMillis);
}

void McuComms::setDebugPort(Stream* dbgPort) {
  McuComms::debugPort = dbgPort;
}

void McuComms::setMessageReceivedCallback(MessageReceivedCallback callback) {
  messageReceivedCallback = callback;
}

void McuComms::sendMessage(McuCommsMessageType messageType) {
  sendMessage(messageType, emptyVector());
}

void McuComms::sendMessage(McuCommsMessageType messageType, const std::vector<uint8_t>& data) {
  if (!isConnected()) return;
  sendMultiPacket(data, messageType);
}

void McuComms::readDataAndTick() {
  uint8_t availableData = transfer.available();

  if (availableData > 0) {
    lastByteReceived = millis();
    auto messageType = static_cast<McuCommsMessageType>(transfer.currentPacketID());
    log("Received a packet [%d]\n", static_cast<uint8_t>(messageType));

    if (messageType == McuCommsMessageType::MCUC_HEARTBEAT) return;
    auto data = receiveMultiPacket();
    if (messageReceivedCallback) {
      messageReceivedCallback(messageType, data);
    }
  }

  if (millis() - lastHeartbeatSent > HEARTBEAT_TIME_DELTA_MSEC) {
    log("McuConnection[%s]: Sending heartbeat.\n", isConnected() ? "active" : "inactive");
    sendHeartbeat();
    lastHeartbeatSent = millis();
  }
}

/** If we haven't heard from the other side for 3 heartbeat times we're no longer connected */
bool McuComms::isConnected() {
  return millis() - lastByteReceived < 3 * HEARTBEAT_TIME_DELTA_MSEC;
}
