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
    if (isForceStarted || outputFlowStarted || state.liquidPumped >= 60.f) {
      outputFlowStarted = true;
      return;
    }

    float pressure = fmax(state.smoothedPressure, 0.f);

    float previousPuckResistance = puckResistance;
    puckResistance = pressure * 1000.f / state.smoothedPumpFlow; // Resistance in mBar * s / g
    resistanceDelta = puckResistance - previousPuckResistance;

    float pressureTarget = phase.getType() == PHASE_TYPE_PRESSURE ? phase.getTarget() : phase.getRestriction();
    pressureTarget = (pressureTarget > 0.f) ? pressureTarget : 12.f;

    // If flow is too big for given pressure or the delta is changing too quickly we're not there yet
    if (resistanceDelta > 500.f || puckResistance < 1500.f) {
      return;
    }

    // If pressure or flow is changing quickly we're not there yet
    if (state.isPressureRisingFast || state.isPumpFlowRisingFast) {
      return;
    }

    // If the pressure hasn't reached its target we're not there yet
    if (pressureTarget - pressure > 0.5f) {
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
