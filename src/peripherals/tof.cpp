#include <Arduino.h>
#include "tof.h"
#include "measurements.h"

TOF::TOF() {};

void TOF::init(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  Wire.begin();
  while(!sensor.tofReady) {
    sensor.tofReady = tof.init();
  }
  tof.startContinuous();
  #endif
}

uint16_t TOF::readLvl(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  static Measurements sensorOutput(20);
  sensorOutput.add(tof.readRangeContinuousMillimeters());
  sensor.tofReading = readRangeToPct(sensorOutput.latest().value);
  #endif
  return sensor.tofReading != 0 ? sensor.tofReading : 125u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  enum WaterLevel {
    Full = 100u,
    LVL90 = 90u,
    LVL80 = 80u,
    LVL70 = 70u,
    LVL60 = 60u,
    LVL50 = 50u,
    LVL40 = 40u,
    LVL30 = 30u,
    LVL20 = 20u,
    LVL10 = 10u,
    Refill = 9u
  };

  static const std::array<uint16_t, 9> ranges = { 15u, 30u, 45u, 60u, 75u, 90u, 105u, 115u, 125u };
  for (size_t i = 0; i < ranges.size(); i++) {
    if (val >= ranges[i]) {
      return WaterLevel(i);
    }
  }

  return WaterLevel::Refill; // if val < 15
}
