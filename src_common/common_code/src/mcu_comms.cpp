#include "mcu_comms.h"

size_t ProfileSerializer::neededBufferSize(Profile& profile) {
  return sizeof(profile.count) + profile.count * sizeof(Phase);
}

uint8_t* ProfileSerializer::serializeProfile(Profile& profile) {
  size_t size = neededBufferSize(profile);
  uint8_t* buffer = new uint8_t[size];

  memcpy(buffer, &profile.count, sizeof(profile.count));
  memcpy(buffer + sizeof(profile.count), profile.phases, profile.count * sizeof(Phase));

  return buffer;
}

void ProfileSerializer::deserializeProfile(const uint8_t* data, Profile& profile) {
  memcpy(&profile.count, data, sizeof(profile.count));
  profile.phases = new Phase[profile.count];
  memcpy(profile.phases, data + sizeof(profile.count), profile.count * sizeof(Phase));
}

//---------------------------------------------------------------------------------
//---------------------------    PRIVATE METHODS       ----------------------------
//---------------------------------------------------------------------------------
void McuComms::sendMultiPacket(uint8_t* buffer, size_t dataSize, uint8_t packetID) {
  log("Sending buffer[%d]: ", dataSize);
  logBufferHex(buffer, dataSize);

  uint8_t dataPerPacket = packetSize - 2; // Two bytes are reserved for current index and last index
  uint8_t numPackets = dataSize / dataPerPacket;

  if (dataSize % dataPerPacket > 0) // Add an extra transmission if needed
    numPackets++;

  for (uint8_t currentPacket = 0; currentPacket < numPackets; currentPacket++) {
    uint8_t dataLen = dataPerPacket;

    if (((currentPacket + 1) * dataPerPacket) > dataSize) // Determine data length for the last packet if file length is not an exact multiple of `dataPerPacket`
      dataLen = dataSize - currentPacket * dataPerPacket;

    uint8_t sendSize = transfer.txObj(numPackets - 1, 0); // index of last packet
    sendSize = transfer.txObj(currentPacket, 1); // index of current packet
    sendSize = transfer.txObj(buffer[currentPacket * dataPerPacket], 2, dataLen); // packet payload

    transfer.sendData(sendSize, packetID); // Send the current file index and data
  }
  log("Data sent.\n");
}

uint8_t* McuComms::receiveMultiPacket() {
  uint8_t lastPacket = transfer.packet.rxBuff[0]; // Get index of last packet
  uint8_t currentPacket = transfer.packet.rxBuff[1]; // Get index of current packet
  uint8_t bytesRead = transfer.bytesRead; // Bytes read in current packet
  uint8_t dataPerPacket = packetSize - 2;
  size_t  totalBytes = 0;

  uint8_t* buffer = new uint8_t[(lastPacket + 1) * dataPerPacket];

  while (currentPacket <= lastPacket) {
#ifdef ESP32
    esp_task_wdt_reset();
#endif

    log("Handling packet %d\n", currentPacket);
    totalBytes += bytesRead - 2;

    // First 2 bytes are not added to the buffer because they represent the index of current and last packet
    for (int i = 0; i < bytesRead - 2; i++) {
      buffer[currentPacket * dataPerPacket + i] = transfer.packet.rxBuff[i + 2];
    }
    if (currentPacket == lastPacket) break;

    // wait for more data to become available for up to 20 milliseconds
    uint8_t dataAvailable = transfer.available();

    uint32_t waitStartedAt = millis();
    while (millis() - waitStartedAt < 50 && !dataAvailable) {
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

  // LOG_INFO("Finished handling packets");
  log("Received buffer[%d]: ", totalBytes);
  logBufferHex(buffer, totalBytes);

  return buffer;
}


void McuComms::shotSnapshotReceived(ShotSnapshot& snapshot) {
  if (shotSnapshotCallback) {
    shotSnapshotCallback(snapshot);
  }
}

void McuComms::profileReceived(Profile& profile) {
  if (profileCallback) {
    profileCallback(profile);
  }
}

void McuComms::log(const char* format, ...) {
  if (!debugPort) return;

  char buffer[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  debugPort->print("McuComms: ");
  debugPort->print(buffer);
}

void McuComms::logBufferHex(uint8_t* buffer, size_t dataSize) {
  if (!debugPort) return;

  for (size_t i = 0; i < dataSize; i++) {
    debugPort->printf("%02x ", buffer[i]);
  }
  debugPort->printf("\n");
}

//---------------------------------------------------------------------------------
//---------------------------    PUBLIC METHODS       ----------------------------
//---------------------------------------------------------------------------------
void McuComms::begin(Stream& serial, size_t packetSize) {
  McuComms::packetSize = packetSize;
  log("Staring with packetSize: %d\n", packetSize);
#ifdef ESP32
  log("Starting for ESP32\n");
#endif
  transfer.begin(serial, true);
}

void McuComms::setDebugPort(Stream* debugPort) {
  McuComms::debugPort = debugPort;
}

void McuComms::setShotSnapshotCallback(ShotSnapshotReceivedCallback callback) {
  shotSnapshotCallback = callback;
}

void McuComms::setProfileReceivedCallback(ProfileReceivedCallback callback) {
  profileCallback = callback;
}


void McuComms::sendShotData(ShotSnapshot& snapshot) {
  uint16_t messageSize = transfer.txObj(snapshot);
  transfer.sendData(messageSize, PACKET_SHOT_SNAPSHOT);
}

void McuComms::sendProfile(Profile& profile) {
  size_t dataSize = profileSerializer.neededBufferSize(profile);
  uint8_t* buffer = profileSerializer.serializeProfile(profile);
  sendMultiPacket(buffer, dataSize, PACKET_PROFILE);
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
      uint8_t* data = receiveMultiPacket();
      Profile profile;
      profileSerializer.deserializeProfile(data, profile);
      delete[] data;
      profileReceived(profile);
      break;
    }
    default:
      log("WARN: Packet ID %d not handled\n", transfer.currentPacketID());
      break;
    }
  }
}
