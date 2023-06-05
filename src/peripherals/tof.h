#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include <SimpleKalmanFilter.h>
#include "../../lib/Common/system_state.h"

// SimpleKalmanFilter smoothTofOutput(1.f, 1.f, 0.01f);

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
  static uint16_t i = 0;
  uint16_t val = 35;
  static uint16_t lastNonNullVal = 36;
  #ifdef TOF_VL53L0X

  val = tof.readRangeResult();
  if (val != 0) {
    lastNonNullVal = val;
    if (i < 1000) sensor.tofReading[i] = val;
    else i = 0;
    return val;
  }
  // val = smoothTofOutput.updateEstimate(val);
  #endif
  return lastNonNullVal;
}

#endif
