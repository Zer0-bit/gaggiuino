#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include "../../lib/Common/sensors_state.h"

class TOF {
  public:
    TOF();
    void begin();
    void readLvl(SensorState &state);

  private:
    Adafruit_VL53L0X tof;
};

TOF::TOF(){};

void TOF::begin() {
  #ifdef TOF_VL53L0X
  if (!tof.begin()) {
    while(1);
  }
  // start continuous ranging
  tof.startRangeContinuous();
  #endif
}

void TOF::readLvl(SensorState &state) {
  #ifdef TOF_VL53L0X
  if (tof.isRangeComplete())
    state.tofReading = tof.readRange();
  #endif
}
#endif
