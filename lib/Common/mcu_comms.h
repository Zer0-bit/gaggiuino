#ifndef MCU_COMMS_H
#define MCU_COMMS_H

#include <Arduino.h>

#include <vector>

#include "SerialTransfer.h"
#include "profiling_phases.h"

#ifdef ESP32
#include "esp_task_wdt.h"
#endif

#define PACKET_SHOT_SNAPSHOT 1
#define PACKET_PROFILE 2
#define PACKET_SENSOR_STATE_SNAPSHOT 3
#define MAX_DATA_PER_PACKET_DEFAULT 58

using namespace std;

class ProfileSerializer {
   public:
    size_t neededBufferSize(Profile& profile);
    vector<uint8_t> serializeProfile(Profile& profile);
    void deserializeProfile(vector<uint8_t>& data, Profile& profile);
};

class McuComms {
   private:
    typedef void (*ShotSnapshotReceivedCallback)(ShotSnapshot&);
    typedef void (*ProfileReceivedCallback)(Profile&);
    typedef void (*SensorStateSnapshotReceivedCallback)(SensorStateSnapshot&);

    ProfileSerializer profileSerializer;
    SerialTransfer transfer;
    ShotSnapshotReceivedCallback shotSnapshotCallback;
    ProfileReceivedCallback profileCallback;
    SensorStateSnapshotReceivedCallback sensorStateSnapshotCallback;
    size_t packetSize;
    Stream* debugPort;

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
    void sendMultiPacket(vector<uint8_t>& buffer, size_t dataSize, uint8_t packetID);
    vector<uint8_t> receiveMultiPacket();
    void log(const char* format, ...);
    void logBufferHex(vector<uint8_t>& buffer, size_t dataSize);

    void shotSnapshotReceived(ShotSnapshot& snapshot);
    void profileReceived(Profile& profile);
    void sensorStateSnapshotReceived(SensorStateSnapshot& snapshot);

   public:
    McuComms() : shotSnapshotCallback(nullptr),
                 profileCallback(nullptr),
                 sensorStateSnapshotCallback(nullptr),
                 debugPort(nullptr){};

    void begin(Stream& serial, size_t packetSize = MAX_DATA_PER_PACKET_DEFAULT);
    void setDebugPort(Stream* debugPort);
    void setShotSnapshotCallback(ShotSnapshotReceivedCallback callback);
    void setProfileReceivedCallback(ProfileReceivedCallback callback);
    void setSensorStateSnapshotCallback(SensorStateSnapshotReceivedCallback callback);

    void sendShotData(ShotSnapshot& snapshot);
    void sendProfile(Profile& profile);
    void sendSensorStateSnapshot(SensorStateSnapshot& snapshot);

    void readData();
};

#endif
