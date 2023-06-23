
#include <Arduino.h>
#include "tof.h"
#include "measurements.h"

TOF::TOF() {};

void TOF::init(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  while(!sensor.tofReady) {
    sensor.tofReady = tof.init();
  }
  tof.startContinuous(10u);
  #endif
}

uint16_t TOF::readLvl(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  static Measurements sensorOutput(20);
  sensorOutput.add(tof.readRangeContinuousMillimeters());
  sensor.tofReading = readRangeToPct(sensorOutput.latest().value);
  #endif
  return sensor.tofReading != 0 ? sensor.tofReading : 135u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  static const uint16_t water_lvl[] = { 100u, 90u, 80u, 70u, 60u, 50u, 40u, 30u, 20u, 10u };
  static const uint16_t ranges[] = { 15u, 30u, 45u, 60u, 75u, 90u, 105u, 120u, 125u };

  for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); i++) {
    if (val <= ranges[i]) {
      return water_lvl[i];
    }
  }

  return 9; // if val < 15
}
