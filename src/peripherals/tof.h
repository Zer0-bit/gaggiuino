#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include <SimpleKalmanFilter.h>
#include "measurements.h"
#include "../../lib/Common/system_state.h"

Measurements sensorOutput(20);

class TOF {
  public:
    TOF(SystemState& state);
    void init(void);
    uint16_t readLvl(void);
    uint16_t readRangeToPct(uint16_t val);

  private:
    Adafruit_VL53L0X tof = Adafruit_VL53L0X();
    SystemState& sensor;
};

TOF::TOF(SystemState& state) : sensor(state) {}

void TOF::init(void) {
  #ifdef TOF_VL53L0X
  // if (!tof.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY))
  while(!sensor.tofReady) {
    sensor.tofReady = tof.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  }
  // start continuous ranging
  tof.startRangeContinuous(200u);
  #endif
}

uint16_t TOF::readLvl(void) {
  #ifdef TOF_VL53L0X
  sensorOutput.add(tof.readRangeResult());
  sensor.tofReading = sensorOutput.latest().value;

  sensor.tofReading = readRangeToPct(sensor.tofReading);
  #endif
  return sensor.tofReading != 0 ? sensor.tofReading : 135u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  static const uint16_t water_lvl[] = { 100, 90, 80, 70, 60, 50, 40, 30, 20, 10 };
  static const uint16_t ranges[] = { 15, 30, 45, 60, 75, 90, 105, 120, 135 };

  for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); i++) {
    if (val <= ranges[i]) {
      return water_lvl[i];
    }
  }

  return 9; // if val < 15
}

#endif
