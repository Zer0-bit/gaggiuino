#ifndef PREDICTIVE_WEIGHT_H
#define PREDICTIVE_WEIGHT_H

#include "../profiling_phases.h"
#include "../sensors_state.h"

class PredictiveWeight {
private:
  bool outputFlowStarted;
  bool isForceStarted;
  float puckResistance;
  float resistanceDelta;

public:
  bool isOutputFlow() {
    return outputFlowStarted;
  }

  void update(SensorState& state, CurrentPhase& phase) {
    // If at least 60ml have been pumped, there has to be output (unless the water is going to the void)
    // No point going through all the below logic if we hardsetting the predictive scales to start counting
    if (isForceStarted || outputFlowStarted || state.liquidPumped >= 60.f) {
      outputFlowStarted = true;
      return;
    }

    float pressure = fmax(state.smoothedPressure, 0.f);

    float previousPuckResistance = puckResistance;
    puckResistance = pressure * 1000.f / state.smoothedPumpFlow; // Resistance in mBar * s / g
    resistanceDelta = puckResistance - previousPuckResistance;

    float pressureTarget = phase.getType() == PHASE_TYPE_PRESSURE ? phase.getTarget() : phase.getRestriction();
    pressureTarget = (pressureTarget > 0.f) ? pressureTarget : 8.f;

    // If flow is too big for given pressure or the delta is changing too quickly we're not there yet
    if (resistanceDelta > 500.f || puckResistance < 1500.f) {
      return;
    }

    // If pressure or flow is changing quickly we're not there yet
    if (state.isPressureRisingFast || state.isPumpFlowRisingFast || state.isPumpFlowFallingFast) {
      return;
    }

    // If the pressure hasn't reached its target when target <= 3bar we're not there yet
    if (pressureTarget <= 2.f &&  pressure < pressureTarget - 0.05f) {
      return;
    }

    // We're there!
    outputFlowStarted = true;
  }

  void setIsForceStarted(bool value) {
    isForceStarted = value;
  }

  void reset() {
    outputFlowStarted = false;
    isForceStarted = false;
    puckResistance = 0.f;
    resistanceDelta = 0.f;
  }
};

#endif
