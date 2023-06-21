#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include "../../lib/Common/sensors_state.h"

class TOF {
  public:
    TOF();
    void init(SensorState& sensor);
    uint16_t readLvl(SensorState& sensor);
    uint16_t readRangeToPct(uint16_t val);

  private:
    Adafruit_VL53L0X tof = Adafruit_VL53L0X();
};

#endif
