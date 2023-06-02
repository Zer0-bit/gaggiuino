#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include "../../lib/Common/system_state.h"


class TOF {
  public:
    TOF(SystemState& state);
    void begin();
    uint16_t readLvl();

  private:
    Adafruit_VL53L0X tof;
    SystemState& sensor;
};

TOF::TOF(SystemState& state) : sensor(state) {}

void TOF::begin() {
  #ifdef TOF_VL53L0X
  sensor.tofReady = tof.begin();
  // start continuous ranging
  if (sensor.tofReady)
    tof.startRangeContinuous();
  #endif
}

uint16_t TOF::readLvl() {
  uint16_t val = 250;
  #ifdef TOF_VL53L0X
  if (tof.isRangeComplete())
    val = tof.readRange();
  #endif
  return val;
}
#endif
