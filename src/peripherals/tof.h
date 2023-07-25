#ifndef TOF_H
#define TOF_H

#include "HardwareTimer.h"
#include <stdint.h> // for uint8_t
#include <Adafruit_VL53L0X.h>
#include <movingAvg.h>
#include "system_state.h"

Adafruit_VL53L0X tof_sensor;
movingAvg mvAvg(4);

class TOF {
  public:
    TOF();
    void init(SystemState&);
    uint16_t readLvl();
    uint16_t readRangeToPct(uint16_t);

  private:
    uint32_t tofReading;
};

TOF::TOF() {}

void TOF::init(SystemState& systemState) {
  #ifdef TOF_VL53L0X
  while(!systemState.tofReady) {
    systemState.tofReady = tof_sensor.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  }
  tof_sensor.startRangeContinuous();
  mvAvg.begin();
  #endif
}

uint16_t TOF::readLvl() {
  #ifdef TOF_VL53L0X
  if(tof_sensor.isRangeComplete()) {
    TOF::tofReading = mvAvg.reading(tof_sensor.readRangeResult());
  }
  #endif
  return  TOF::tofReading != 0 ? readRangeToPct(TOF::tofReading) : 30u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  static const std::array<uint16_t, 10> water_lvl = { 100u, 90u, 80u, 70u, 60u, 50u, 40u, 30u, 20u, 10u };
  static const std::array<uint16_t, 9> ranges = { 15u, 30u, 45u, 60u, 75u, 90u, 105u, 115u, 125u };
  for (size_t i = 0; i < ranges.size(); i++) {
    if (val <= ranges[i]) {
      return water_lvl[i];
    }
  }

  return 9u;
}

#endif
