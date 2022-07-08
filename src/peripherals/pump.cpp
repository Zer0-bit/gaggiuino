#include "pump.h"
#include "pindef.h"
#include <PSM.h>

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 2, 4);

float flowPerClickAtZeroBar = 0.2628f;
float flowSlopeConstant = 0.01467f;
short maxPumpClicksPerSecond = 50;

// Initialising some pump specific specs, mainly:
// - max pump clicks(dependant on region power grid spec)
// - pump clicks at 0 pressure in the system
void pumpInit(int powerLineFrequency) {
  maxPumpClicksPerSecond = powerLineFrequency;
  flowPerClickAtZeroBar = 50 * flowPerClickAtZeroBar / powerLineFrequency;
}

// Function that returns the percentage of clicks the pump makes in it's current phase
int getPumpPct(float livePressure, float targetValue, float flow, bool isPressureFalling) {
  if (targetValue == 0) {
    return 0;
  }

  float diff = targetValue - livePressure;
  float pumpPctToMaintainFlow = getClicksForFlow(fmin(flow, 5.f), (targetValue + livePressure) / 2.f) / (float) maxPumpClicksPerSecond;

  if (diff > 0) {
    return pumpPctToMaintainFlow + 100 / (1.f + exp(1.5f - diff/1.4f));
  }

  if (diff <= 0 &&  isPressureFalling) {
    return pumpPctToMaintainFlow / 2.f + 100.f / (1.f + exp(2.f - diff/0.2f));
  }

  return 0;
}

// Sets the pump output based on a couple input params:
// - live system pressure
// - expected target
// - flow
// - pressure direction
void setPumpPressure(float livePressure, float targetValue, float flow, bool isPressureFalling) {
  int pumpPct = fmin(100.f, getPumpPct(livePressure, targetValue, flow, isPressureFalling));

  pump.set(pumpPct * PUMP_RANGE / 100);
}

void setPumpOff() {
  pump.set(0);
}

void setPumpToRawValue(uint8_t val) {
  pump.set(val);
}

long getAndResetClickCounter(void) {
  long counter = pump.getCounter();
  pump.resetCounter();
  return counter;
}

// Follows the schematic from http://ulka-ceme.co.uk/E_Models.html modified to per-click
float getPumpFlow(long clickCount, float pressure) {
  return clickCount * (flowPerClickAtZeroBar - pressure * flowSlopeConstant);
}

long getClicksForFlow(float flow, float pressure) {
  return flow / getPumpFlow(1, pressure);
}
