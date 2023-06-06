#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include <SimpleKalmanFilter.h>
#include "../../lib/Common/system_state.h"

SimpleKalmanFilter smoothTofOutput(0.1f, 0.1f, 0.1f);

class TOF {
  public:
    TOF(SystemState& state);
    void init();
    uint16_t readLvl();

  private:
    Adafruit_VL53L0X tof = Adafruit_VL53L0X();
    SystemState& sensor;
};

TOF::TOF(SystemState& state) : sensor(state) {}

void TOF::init() {
  #ifdef TOF_VL53L0X
  // if (!tof.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY))
  while(!sensor.tofReady) {
    sensor.tofReady = tof.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  }
  // start continuous ranging
  tof.startRangeContinuous(200u);
  #endif
}

uint16_t TOF::readLvl() {
  static uint16_t lastNonNullVal = 60;
  #ifdef TOF_VL53L0X

  sensor.tofReading = tof.readRangeResult();
  if (sensor.tofReading != 0) {
    lastNonNullVal = sensor.tofReading;
    return sensor.tofReading;
  }
  sensor.tofReading = smoothTofOutput.updateEstimate(sensor.tofReading);
  #endif
  return lastNonNullVal;
}

#endif
