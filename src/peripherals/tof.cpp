#include <Wire.h>
#include "tof.h"
#include "measurements.h"

TOF::TOF() {};

void TOF::init(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  Wire.begin();
  tof.setAddress(0x29);
  while(!sensor.tofReady) {
    sensor.tofReady = tof.init();
  }
  tof.setMeasurementTimingBudget(200000);
  tof.startContinuous(100u);
  #endif
}

uint16_t TOF::readLvl(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  uint16_t tofReading;
  static Measurements sensorOutput(20);
  sensorOutput.add(tof.readRangeContinuousMillimeters());
  tofReading = readRangeToPct(sensorOutput.latest().value);
  #endif
  return tofReading != 0 ? tofReading : 125u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  static const std::array<uint16_t, 10> water_lvl = { 100u, 90u, 80u, 70u, 60u, 50u, 40u, 30u, 20u, 10u };
  static const std::array<uint16_t, 9> ranges = { 15u, 30u, 45u, 60u, 75u, 90u, 105u, 115u, 125u };
  for (size_t i = 0; i < ranges.size(); i++) {
    if (val <= ranges[i]) {
      return water_lvl[i];
    }
  }

  return 9u; // if val < 15
}
