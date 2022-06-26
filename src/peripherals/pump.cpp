#include "pump.h"
#include "pindef.h"
#include <PSM.h>

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE);

void setPumpPressure(float livePressure, float targetValue) {
  int pumpValue;

  if (targetValue == 0 || livePressure > targetValue) {
    pumpValue = 0;
  } else {
    float diff = targetValue - livePressure;
    pumpValue = PUMP_RANGE / (1.f + exp(1.7f - diff/0.9f));
  }

  pump.set(pumpValue);
}

void setPumpOff() {
  pump.set(0);
}

void setPumpToRawValue(uint8_t val) {
  pump.set(val);
}
