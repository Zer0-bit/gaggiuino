#include "pump.h"
#include "pindef.h"
#include <PSM.h>

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE);

int getPumpPct(float livePressure, float targetValue, bool isPressureFalling) {
  if (targetValue == 0) {
    return 0;
  }

  float diff = targetValue - livePressure;

  if (diff > 0) {
    return targetValue + 100 / (1.f + exp(1.5f - diff/1.4f));
  }

  if (diff <= 0 &&  isPressureFalling) {
    return 100.f / (1.f + exp(2.f - diff/0.2f));
  }

  return 0;
}

void setPumpPressure(float livePressure, float targetValue, bool isPressureFalling) {
  int pumpPct = getPumpPct(livePressure, targetValue, isPressureFalling);

  pump.set(pumpPct * PUMP_RANGE / 100);
}

void setPumpOff() {
  pump.set(0);
}

void setPumpToRawValue(uint8_t val) {
  pump.set(val);
}
