
#include <Arduino.h>
#include "tof.h"
#include "measurements.h"

Measurements sensorOutput(20);

TOF::TOF() {};

void TOF::init(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  while(!sensor.tofReady) {
    sensor.tofReady = tof.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  }
  tof.startRangeContinuous(10u);
  #endif
}

uint16_t TOF::readLvl(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  sensorOutput.add(tof.readRangeResult());
  sensor.tofReading = readRangeToPct(sensorOutput.latest().value);
  #endif
  return sensor.tofReading != 0 ? sensor.tofReading : 135u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  static const uint16_t water_lvl[] = { 100, 90, 80, 70, 60, 50, 40, 30, 20, 10 };
  static const uint16_t ranges[] = { 15, 30, 45, 60, 75, 90, 105, 120, 125 };

  for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); i++) {
    if (val <= ranges[i]) {
      return water_lvl[i];
    }
  }

  return 9; // if val < 15
}
