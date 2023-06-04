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
    void begin();
    uint16_t readLvl();

  private:
    Adafruit_VL53L0X tof;
    SystemState& sensor;
};

TOF::TOF(SystemState& state) : sensor(state) {}

void TOF::begin() {
  #ifdef TOF_VL53L0X
  sensor.tofReady = tof.begin(Adafruit_VL53L0X::VL53L0X_Sense_config_t::VL53L0X_SENSE_HIGH_ACCURACY);
  // start continuous ranging
  if (sensor.tofReady)
    tof.startRangeContinuous();
  #endif
}

uint16_t TOF::readLvl() {
  uint16_t val = 175;
  #ifdef TOF_VL53L0X
  if (tof.isRangeComplete())
    val = tof.readRange();
    val = smoothTofOutput.updateEstimate(val);
  #endif
  return val;
}

#endif
