#include "pump.h"
#include "pindef.h"
#include <PSM.h>

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 2, 4);


float cpsInefficiencyConstant1 = 0.00473f;
float cpsInefficiencyConstant2 = 0.000046f;
float pressureInefficiencyConstant = 0.01467f;
float flowPerClickAtZeroBar = 0.296f;
short maxPumpClicksPerSecond = 50;

// Initialising some pump specific specs, mainly:
// - max pump clicks(dependant on region power grid spec)
// - pump clicks at 0 pressure in the system
void pumpInit(int powerLineFrequency, float pumpFlowAtZero) {
  maxPumpClicksPerSecond = powerLineFrequency;
  flowPerClickAtZeroBar = 50 * pumpFlowAtZero / powerLineFrequency;
}

// Function that returns the percentage of clicks the pump makes in it's current phase
int getPumpPct(float targetPressure, float flowRestriction, SensorState &currentState) {
  if (targetPressure == 0) {
    return 0;
  }

  float diff = targetPressure - currentState.pressure;
  float maxPumpPct = flowRestriction > 0 ? 100.f * getClicksPerSecondForFlow(flowRestriction, currentState.pressure) / maxPumpClicksPerSecond : 100.f;
  float pumpPctToMaintainFlow = 100.f * getClicksPerSecondForFlow(currentState.pumpFlow, currentState.pressure) / maxPumpClicksPerSecond;

  if (diff > 0) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow + fmin(100.f, 25 + 20 * diff));
  }

  if (diff <= 0 &&  currentState.isPressureFalling) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.2f + fmax(0.f, 10 + 20 * diff));
  }

  return 0;
}

// Sets the pump output based on a couple input params:
// - live system pressure
// - expected target
// - flow
// - pressure direction
void setPumpPressure(float targetPressure, float flowRestriction, SensorState &currentState) {
  int pumpPct = fmin(100.f, getPumpPct(targetPressure, flowRestriction, currentState));

  pump.set(pumpPct * PUMP_RANGE / 100);
}

void setPumpOff(void) {
  pump.set(0);
}

void setPumpFullOn(void) {
  pump.set(PUMP_RANGE);
}

void setPumpToRawValue(uint8_t val) {
  pump.set(val);
}

long getAndResetClickCounter(void) {
  long counter = pump.getCounter();
  pump.resetCounter();
  return counter;
}

// Models the flow per click taking into account
// - the pump inefficiency due to pressure (linear)
// - the pump inefficiency due to higher cps (quadratic)
float getFlowPerClick(float pressure, float cps) {
  return flowPerClickAtZeroBar
        - (cpsInefficiencyConstant1 - cpsInefficiencyConstant2 * cps) * cps
        - pressureInefficiencyConstant * pressure;
}

// Follows the schematic from http://ulka-ceme.co.uk/E_Models.html modified to per-click
float getPumpFlow(float cps, float pressure) {
  return cps * getFlowPerClick(pressure, cps);
}

// Binary search for correct CPS in the range of 0,maxCps
// This is needed to find the solution for CPS of the cubicfunction cps * flowPerClick(pressure, cps)
// This loop always finds the solution in log2(maxCps) iterations ~= 7 iterations.
float getClicksPerSecondForFlow(float flow, float pressure) {
  float minCps = 0;
  float maxCps = maxPumpClicksPerSecond;
  float accuracy = 0.5f;
  float cps = 0;

  while (minCps <= maxCps) {
    cps = (minCps + maxCps) / 2.f;
    float estFlow = cps * getFlowPerClick(pressure, cps);
    if (estFlow == flow) {
      return cps;
    } else if (estFlow < flow) {
      minCps = cps + accuracy;
    } else {
      maxCps = cps - accuracy;
    }
  }
  return cps;
}

// Calculates pump percentage for the requested flow and updates the pump raw value
void setPumpFlow(float targetFlow, float pressureRestriction, SensorState &currentState) {
  // If a pressure restriction exists then the we go into pressure profile with a flowRestriction
  // which is equivalent but will achieve smoother pressure management
  if (pressureRestriction > 0) {
    setPumpPressure(pressureRestriction, targetFlow, currentState);
  } else {
    float pumpPct = getClicksPerSecondForFlow(targetFlow, currentState.pressure) / (float) maxPumpClicksPerSecond;
    setPumpToRawValue(pumpPct * PUMP_RANGE);
  }
}
