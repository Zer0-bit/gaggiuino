#include "pump.h"
#include "pindef.h"
#include <PSM.h>
#include "../utils.h"

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 2, 4);

float pressureInefficiencyConstant1 = 0.1224f;
// float pressureInefficiencyConstant2 = 0.01052f;
// float pressureInefficiencyConstant3 = 0.00401f;
// float pressureInefficiencyConstant4 = 0.000705f;
// float pressureInefficiencyConstant5 = 0.00002916f;

// float pressureInefficiencyConstant2 = 0.0105f;
// float pressureInefficiencyConstant3 = 0.0043f;
// float pressureInefficiencyConstant4 = 0.00065f;
// float pressureInefficiencyConstant5 = 0.000039f;
float pressureInefficiencyConstant2 = 0.0038f;
float pressureInefficiencyConstant3 = 0.0015f;
float pressureInefficiencyConstant4 = 0.001216f;
float pressureInefficiencyConstant5 = 0.000087f;
float flowPerClickAtZeroBar = 0.152f;
short maxPumpClicksPerSecond = 50;

// Initialising some pump specific specs, mainly:
// - max pump clicks(dependant on region power grid spec)
// - pump clicks at 0 pressure in the system
void pumpInit(int powerLineFrequency, float pumpFlowAtZero) {
    maxPumpClicksPerSecond = powerLineFrequency;
    flowPerClickAtZeroBar = pumpFlowAtZero;
}

// Function that returns the percentage of clicks the pump makes in it's current phase
float getPumpPct(float targetPressure, float flowRestriction, SensorState &currentState) {
    if (targetPressure == 0.f) {
        return 0.f;
    }

    float diff = targetPressure - currentState.pressure;
    float maxPumpPct = flowRestriction <= 0.f ? 1.f : getClicksPerSecondForFlow(flowRestriction, currentState.smoothedPressure) / (float) maxPumpClicksPerSecond;
    float pumpPctToMaintainFlow = getClicksPerSecondForFlow(currentState.smoothedPumpFlow, currentState.smoothedPressure) / (float) maxPumpClicksPerSecond;

    if (diff > 2.f) {
      return fminf(maxPumpPct, 0.25f + 0.2f * diff);
    }

    if (diff > 0.f) {
        return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.95f + 0.1f + 0.2f * diff);
    }

    if (diff <= 0.f && currentState.isPressureFalling) {
        return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.5f);
    }

    return 0;
}

// Sets the pump output based on a couple input params:
// - live system pressure
// - expected target
// - flow
// - pressure direction
void setPumpPressure(float targetPressure, float flowRestriction, SensorState &currentState) {
    float pumpPct = getPumpPct(targetPressure, flowRestriction, currentState);

    pump.set(pumpPct * PUMP_RANGE);
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

// Models the flow per click
// Follows a compromise between the schematic and recorded findings
//
// The function is split to compensate for the rapid decline in fpc at low pressures
float getFlowPerClick(float pressure) {
    float fpc;
    if (pressure >= 0.f && pressure <= 6.f) {
      fpc = mapRange(pressure, 0.f, 6.f, 640.f, 350.f, 1) / 50 / maxPumpClicksPerSecond;
    } else if (pressure > 6.f && pressure <= 8.f) {
      fpc = mapRange(pressure, 6.f, 8.f, 351.f, 300.f, 1) / 50 / maxPumpClicksPerSecond;
    } else if (pressure > 8.f && pressure <= 12.f) {
      fpc = mapRange(pressure, 8.f, 12.f, 301.f, 105.f, 1) / 50 / maxPumpClicksPerSecond;
    } else {
      fpc = mapRange(pressure, 12.f, 16.f, 105.f, 0.1f, 1) / 50 / maxPumpClicksPerSecond;
    }
    fpc = constrain(fpc, 0.f, 0.24f);
    return (fpc + (pressure < 5.f ? flowPerClickAtZeroBar+flowPerClickAtZeroBar*0.5f : flowPerClickAtZeroBar*0.9f)) / 2.f;
}

// Follows the schematic from http://ulka-ceme.co.uk/E_Models.html modified to per-click
float getPumpFlow(float cps, float pressure) {
    return cps * getFlowPerClick(pressure);
}

// Currently there is no compensation for pressure measured at the puck, resulting in incorrect estimates
float getClicksPerSecondForFlow(float flow, float pressure) {
    float flowPerClick = getFlowPerClick(pressure);
    float cps = flow / flowPerClick;
    return fminf(cps, maxPumpClicksPerSecond);
}

// Calculates pump percentage for the requested flow and updates the pump raw value
void setPumpFlow(float targetFlow, float pressureRestriction, SensorState &currentState) {
    // If a pressure restriction exists then the we go into pressure profile with a flowRestriction
    // which is equivalent but will achieve smoother pressure management
    if (pressureRestriction > 0) {
        setPumpPressure(pressureRestriction, targetFlow, currentState);
    } else {
        float pumpPct = getClicksPerSecondForFlow(targetFlow, currentState.smoothedPressure) / (float)maxPumpClicksPerSecond;
        setPumpToRawValue(pumpPct * PUMP_RANGE);
    }
}
