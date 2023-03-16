/* 09:32 15/03/2023 - change triggering comment */
#ifndef PREDICTIVE_WEIGHT_H
#define PREDICTIVE_WEIGHT_H

#include "profiling_phases.h"
#include "sensors_state.h"
#include "../eeprom_data/eeprom_data.h"

constexpr float crossSectionalArea = 0.0026f; // avg puck crossectional area.
constexpr float dynamicViscosity = 0.0002964f; // avg water dynamic viscosity at 90-95 celsius.

class PredictiveWeight {
private:
  bool outputFlowStarted;
  bool isForceStarted;
  float puckResistance;
  float truePuckResistance;
  float resistanceDelta;
  float pressureDrop;

public:
  PredictiveWeight() :
    outputFlowStarted(false),
    isForceStarted(false),
    puckResistance(0.f),
    truePuckResistance(0.f),
    resistanceDelta(0.f),
    pressureDrop(0.f)
  {}

  bool isOutputFlow() {
    return outputFlowStarted;
  }

  inline float calculatePuckResistance(float waterFlowRate, float crossSectionalArea, float dynamicViscosity, float pressureDrop)
  {
    float resistance = -(dynamicViscosity * waterFlowRate) / (crossSectionalArea * pressureDrop);
    return resistance;
  }

  void update(const SensorState& state, CurrentPhase& phase, const eepromValues_t& cfg) {
    // If at least 50ml have been pumped, there has to be output (unless the water is going to the void)
    // No point going through all the below logic if we hardsetting the predictive scales to start counting
    if (isForceStarted || outputFlowStarted || state.waterPumped >= 65.f) {
      outputFlowStarted = true;
      return;
    }
    float previousPuckResistance = puckResistance;
    puckResistance = state.smoothedPressure * 1000.f / state.smoothedPumpFlow; // Resistance in mBar * s / g
    resistanceDelta = puckResistance - previousPuckResistance;
    pressureDrop = state.smoothedPressure * 10.f;
    pressureDrop -= pressureDrop - state.pumpClicks;
    pressureDrop = pressureDrop > 0.f ? pressureDrop : 1.f;
    truePuckResistance = calculatePuckResistance(state.smoothedPumpFlow, crossSectionalArea, dynamicViscosity, pressureDrop);

    // Through empirical testing it's been observed that ~2 bars is the indicator of the pf headspace being full
    // as well as there being enough pressure for water to wet the puck enough to start the output
    bool phaseTypePressure = phase.getType() == PHASE_TYPE::PHASE_TYPE_PRESSURE;
    // float pressureTarget = phaseTypePressure ? phase.getTarget() : phase.getRestriction();
    // pressureTarget = (pressureTarget == 0.f || pressureTarget > 2.f) ? 2.f : pressureTarget;
    // We need to watch when pressure goes above the PI pressure which is a better indicator of headspace being filled.
    // float preinfusionPressure = cfg.preinfusionFlowState ? cfg.preinfusionFlowPressureTarget : cfg.preinfusionBar;
    // Pressure has to cross the 1 bar threshold.
    if (state.smoothedPressure < 2.1f) return;

    if (phaseTypePressure) {
      // If the pressure or flow are raising too fast dismiss the spike from the output.
      if (state.isPressureRisingFast || state.isPumpFlowRisingFast) return;
      // If flow is too big for given pressure or the delta is changing too quickly we're not there yet
      if (resistanceDelta > 500.f || puckResistance < 1100.f) return;
    }

    // If flow is too big for given pressure or the delta is changing too quickly we're not there yet
    // if (puckResistance < 1100.f) return;
    if (truePuckResistance < -0.015f) return;

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
