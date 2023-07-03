/* 09:32 15/03/2023 - change triggering comment */
#ifndef PREDICTIVE_WEIGHT_H
#define PREDICTIVE_WEIGHT_H

#include "profiling_phases.h"
#include "sensors_state.h"
#include "shot_profiler.h"

extern int preInfusionFinishedPhaseIdx;
constexpr float crossSectionalArea = 0.0026f; // avg puck crossectional area.
constexpr float dynamicViscosity = 0.0002964f; // avg water dynamic viscosity at 90-95 celsius.
bool predictiveTargetReached = false;

class PredictiveWeight {
private:
  bool outputFlowStarted;
  bool isForceStarted;
  float puckResistance;
  float truePuckResistance;
  float resistanceDelta;
  float pressureDrop;
  float peakPressure;

public:
  bool preinfusionFinished;
  PredictiveWeight() :
    outputFlowStarted(false),
    isForceStarted(false),
    puckResistance(0.f),
    truePuckResistance(0.f),
    resistanceDelta(0.f),
    pressureDrop(0.f) {
  }

  bool isOutputFlow() {
    return outputFlowStarted;
  }

  inline float calculatePuckResistance(float waterFlowRate, float crossSectionalArea, float dynamicViscosity, float pressureDrop) {
    float resistance = -(dynamicViscosity * waterFlowRate) / (crossSectionalArea * pressureDrop);
    return resistance;
  }

  void update(const SensorState& state, CurrentPhase& phase, const GaggiaSettings& cfg) {
    // If at least 50ml have been pumped, there has to be output (unless the water is going to the void)
    // No point going through all the below logic if we hardsetting the predictive scales to start counting
    if (isForceStarted || outputFlowStarted || state.waterPumped >= 65.f) {
      outputFlowStarted = true;
      return;
    }
    float previousPuckResistance = puckResistance;

    puckResistance = state.smoothedPressure * 1000.f / state.smoothedPumpFlow; // Resistance in mBar * s / g
    resistanceDelta = puckResistance - previousPuckResistance;
    peakPressure = fmaxf(peakPressure, state.smoothedPressure);
    pressureDrop = state.smoothedPressure * 10.f;
    pressureDrop -= pressureDrop - state.pumpClicks;
    pressureDrop = pressureDrop > 0.f ? pressureDrop : 1.f;
    truePuckResistance = calculatePuckResistance(state.smoothedPumpFlow, crossSectionalArea, dynamicViscosity, pressureDrop);

    /* ::OBSERVATIONS::
    Through empirical testing it's been observed that ~2 bars is the indicator of the pf headspace being full
    as well as there being enough pressure for water to wet the puck enough to start the output.
    On profiles whare pressure drop is of concern ~1 bar of drop is the point where liquid output starts. */

    // The following should catch soak phase dripping. We've reached a peak that is over 2bars and the pressure
    // is now dropping. Has dropped at least 1 bar.
    if (peakPressure > 2.f && state.smoothedPressure < peakPressure - 1.f) {
      outputFlowStarted = true;
    }

    // Pressure has to cross the 2 bar threshold.
    if (state.smoothedPressure < 2.1f) return;

    if (phase.getType() == PhaseType::PRESSURE) {
      // If the pressure or flow are raising too fast dismiss the spike from the output.
      if (fabsf(state.pressureChangeSpeed) > 5.f || fabsf(state.pumpFlowChangeSpeed) > 2.f) return;
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
    peakPressure = 0.f;
    puckResistance = 0.f;
    resistanceDelta = 0.f;
    isForceStarted = false;
    outputFlowStarted = false;
    predictiveTargetReached = false;
  }
};

#endif
