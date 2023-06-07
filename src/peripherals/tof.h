#ifndef TOF_H
#define TOF_H

#include <stdint.h> // for uint8_t
#include "Adafruit_VL53L0X.h"
#include <SimpleKalmanFilter.h>
#include "../../lib/Common/system_state.h"

SimpleKalmanFilter smoothTofOutput(0.01f, 0.01f, 0.01f);

class TOF {
  public:
    TOF(SystemState& state);
    void init(void);
    uint16_t readLvl(void);
    uint16_t avgLvl(uint16_t val);

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
  static uint16_t lastNonNullVal;
  sensor.tofReading = tof.readRangeResult();
  if (sensor.tofReading != 0) {
    lastNonNullVal = sensor.tofReading;
    sensor.tofReading = smoothTofOutput.updateEstimate(sensor.tofReading);
    return sensor.tofReading;
  } else sensor.tofReading = lastNonNullVal;
  // sensor.tofReading = avgLvl(sensor.tofReading);
  #endif
  return sensor.tofReading != 0 ? sensor.tofReading : 60;
}

uint16_t TOF::avgLvl(uint16_t val) {
  if (val >= 149) val = 10;
  else if (val >= 133) val = 20;
  else if (val >= 117) val = 30;
  else if (val >= 100) val = 40;
  else if (val >= 85) val = 50;
  else if (val >= 69) val = 60;
  else if (val >= 53) val = 70;
  else if (val >= 37) val = 80;
  else if (val >= 20) val = 90;
  else val = 100;

  return val;
}

#endif
