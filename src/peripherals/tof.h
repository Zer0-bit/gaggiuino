#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include "../../lib/Common/sensors_state.h"

class TOF {
  public:
    TOF();
    void begin();
    uint16_t readLvl();

  private:
    Adafruit_VL53L0X tof;
};

TOF::TOF(){};

void TOF::begin() {
  #ifdef TOF_VL53L0X
  if (!tof.begin()) {
    return;
  }
  // start continuous ranging
  tof.startRangeContinuous();
  #endif
}

uint16_t TOF::readLvl() {
  uint16_t val = 25000;
  #ifdef TOF_VL53L0X
  if (tof.isRangeComplete())
    val = tof.readRange();
  #endif
  return val;
}
#endif
