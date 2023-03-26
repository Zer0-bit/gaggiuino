/* 09:32 15/03/2023 - change triggering comment */
#include "mcu_comms.h"
#include <stdarg.h>

using namespace std;

size_t ProfileSerializer::neededBufferSize(Profile& profile) const {
  return sizeof(profile.phaseCount()) + profile.phaseCount() * sizeof(Phase) + sizeof(profile.globalStopConditions);
}

vector<uint8_t> ProfileSerializer::serializeProfile(Profile& profile) {
  vector<uint8_t> buffer;
  buffer.reserve(neededBufferSize(profile));
  size_t phaseCount = profile.phaseCount();

  memcpy(buffer.data(), &phaseCount, sizeof(phaseCount));
  memcpy(buffer.data() + sizeof(phaseCount), profile.phases.data(), phaseCount * sizeof(Phase));
  memcpy(buffer.data() + sizeof(phaseCount) + phaseCount * sizeof(Phase), &profile.globalStopConditions, sizeof(profile.globalStopConditions));

  return buffer;
}

void ProfileSerializer::deserializeProfile(vector<uint8_t>& buffer, Profile& profile) const{
  size_t phaseCount;
  memcpy(&phaseCount, buffer.data(), sizeof(profile.phaseCount()));
  profile.phases.clear();
  profile.phases.reserve(phaseCount);
  memcpy(profile.phases.data(), buffer.data() + sizeof(profile.phaseCount()), phaseCount * sizeof(Phase));
  memcpy(&profile.globalStopConditions, buffer.data() + sizeof(profile.phaseCount()) + phaseCount * sizeof(Phase), sizeof(profile.globalStopConditions));
}

//---------------------------------------------------------------------------------
//---------------------------    PRIVATE METHODS       ----------------------------
//---------------------------------------------------------------------------------
void McuComms::sendMultiPacket(vector<uint8_t>& buffer, size_t dataSize, uint8_t packetID) {
  log("Sending buffer[%d]: ", dataSize);
  logBufferHex(buffer, dataSize);

  auto dataPerPacket = static_cast<uint8_t>(packetSize - 2u); // Two bytes are reserved for current index and last index
  auto numPackets = static_cast<uint8_t>(dataSize / dataPerPacket);

  if (dataSize % dataPerPacket > 0u) // Add an extra transmission if needed
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


void McuComms::shotSnapshotReceived(ShotSnapshot& snapshot) const {
  if (shotSnapshotCallback) {
    shotSnapshotCallback(snapshot);
  }
}

void McuComms::profileReceived(Profile& profile) const {
  if (profileCallback) {
    profileCallback(profile);
  }
}

void McuComms::sensorStateSnapshotReceived(SensorStateSnapshot& snapshot) const {
  if (sensorStateSnapshotCallback) {
    sensorStateSnapshotCallback(snapshot);
  }
}

void McuComms::log(const char* format, ...) {
  if (!debugPort) return;

  std::array<char,128>buffer;
  va_list args;
  va_start(args, format);
  vsnprintf(buffer.data(), buffer.size(), format, args);
  va_end(args);
  debugPort->print("McuComms: ");
  debugPort->print(buffer.data());
}

void McuComms::logBufferHex(vector<uint8_t>& buffer, size_t dataSize) {
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

//---------------------------------------------------------------------------------
//---------------------------    PUBLIC METHODS       ----------------------------
//---------------------------------------------------------------------------------
void McuComms::begin(Stream& serial, size_t pcktSize) {
  McuComms::packetSize = pcktSize;
  log("Staring with packetSize: %d\n", pcktSize);
#ifdef ESP32
  log("Starting for ESP32\n");
#endif
  transfer.begin(serial, true);
}

void McuComms::setDebugPort(Stream* dbgPort) {
  McuComms::debugPort = dbgPort;
}

void McuComms::setShotSnapshotCallback(ShotSnapshotReceivedCallback callback) {
  shotSnapshotCallback = callback;
}

void McuComms::setProfileReceivedCallback(ProfileReceivedCallback callback) {
  profileCallback = callback;
}

void McuComms::setSensorStateSnapshotCallback(SensorStateSnapshotReceivedCallback callback) {
  sensorStateSnapshotCallback = callback;
}

void McuComms::sendShotData(const ShotSnapshot& snapshot) {
  uint16_t messageSize = transfer.txObj(snapshot);
  transfer.sendData(messageSize, PACKET_SHOT_SNAPSHOT);
}

void McuComms::sendProfile(Profile& profile) {
  size_t dataSize = profileSerializer.neededBufferSize(profile);
  vector<uint8_t> buffer = profileSerializer.serializeProfile(profile);
  sendMultiPacket(buffer, dataSize, PACKET_PROFILE);
}

void McuComms::sendSensorStateSnapshot(const SensorStateSnapshot& snapshot) {
  uint16_t messageSize = transfer.txObj(snapshot);
  transfer.sendData(messageSize, PACKET_SENSOR_STATE_SNAPSHOT);
}

void McuComms::readData() {
  size_t availableData = transfer.available();

  if (availableData) {
    log("Some data is available\n");

    switch (transfer.currentPacketID()) {
    case PACKET_SHOT_SNAPSHOT: {
      log("Received a shot snapshot packet\n");
      ShotSnapshot snapshot;
      transfer.rxObj(snapshot);
      shotSnapshotReceived(snapshot);
      break;
    } case PACKET_PROFILE: {
      log("Received a profile packet\n");
      vector<uint8_t> data = receiveMultiPacket();
      Profile profile;
      profileSerializer.deserializeProfile(data, profile);
      profileReceived(profile);
      break;
    } case PACKET_SENSOR_STATE_SNAPSHOT: {
      log("Received a sensor state snapshot packet\n");
      SensorStateSnapshot snapshot;
      transfer.rxObj(snapshot);
      sensorStateSnapshotReceived(snapshot);
      break;
    }
    default:
      log("WARN: Packet ID %d not handled\n", transfer.currentPacketID());
      break;
    }
  }
}
