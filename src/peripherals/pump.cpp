#include "pump.h"
#include "pindef.h"
#include <PSM.h>
#include "../utils.h"

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 2, 4);

// float pressureInefficiencyConstant1 = 0.1224f;
// float pressureInefficiencyConstant2 = -0.01052f;
// float pressureInefficiencyConstant3 = -0.00401f;
// float pressureInefficiencyConstant4 = 0.00066f;
// float pressureInefficiencyConstant5 = -0.0000305f;

//ATTEMPT1
// float pressureInefficiencyConstant2 = -0.00678f;
// float pressureInefficiencyConstant3 = -0.00335f;
// float pressureInefficiencyConstant4 = 0.000448f;
// float pressureInefficiencyConstant5 = -0.0000173f;

//ATTEMPT2
float pressureInefficiencyConstant1 = 0.11f;
float pressureInefficiencyConstant2 = 0.000865f;
float pressureInefficiencyConstant3 = -0.00254f;
float pressureInefficiencyConstant4 = 0.000249f;
float pressureInefficiencyConstant5 = -0.00000907f;

float flowPerClickAtZeroBar = 0.29f;
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
    if (pressure <= 0.5f) {
        fpc = flowPerClickAtZeroBar - pressureInefficiencyConstant1 * pressure - 0.061f;
    } else {
        fpc = (flowPerClickAtZeroBar - 0.116f) + (pressureInefficiencyConstant2 + (pressureInefficiencyConstant3 + (pressureInefficiencyConstant4 + pressureInefficiencyConstant5 * pressure) * pressure) * pressure) * pressure;
    }

    return 50.0f * fpc / (float)maxPumpClicksPerSecond;
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
