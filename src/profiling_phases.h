#ifndef PROFILING_PHASES_H
#define PROFILING_PHASES_H

#include "utils.h"
#include "sensors_state.h"
#include "eeprom_data.h"

enum PHASE_TYPE {
  PHASE_TYPE_FLOW,
  PHASE_TYPE_PRESSURE
};

enum STAGE_TYPE {
  STAGE_PI_FILL,
  STAGE_PI_SOAK,
  STAGE_PI_RAMP,
  STAGE_PP_HOLD,
  STAGE_PP_MAIN,
  STAGE_FP_MAIN,
};

struct Phase {
    PHASE_TYPE type;
    STAGE_TYPE stage;
    float startValue;
    float endValue;
    float startRestriction;
    float endRestriction;
    unsigned int durationMs;
    float shotTarget;
    float pressureTarget;

    float getTarget(unsigned long timeInPhase) {
      return mapRange(timeInPhase, 0, durationMs, startValue, endValue, 1);
    }

    float getRestriction(unsigned long timeInPhase) {
      return mapRange(timeInPhase, 0, durationMs, startRestriction, endRestriction, 1);
    }

    float getTimeRestriction(SensorState& currentState, eepromValues_t& runningCfg) {
      float timeMs;
      if (runningCfg.flowProfileState) {
        timeMs = mapRange(currentState.smoothedPressure, 0, runningCfg.preinfusionFlowPressureTarget-0.05f, runningCfg.preinfusionFlowTime, 0, 1);
        timeMs = constrain(timeMs, 0, runningCfg.preinfusionFlowTime-0.5f);
        return timeMs;
      }
      else {
        timeMs = mapRange(currentState.smoothedPressure, 0, runningCfg.preinfusionBar ,runningCfg.preinfusionFlowTime-0.05f, 0, 1);
        timeMs = constrain(timeMs, 0, runningCfg.preinfusionFlowTime-0.5f);
        return timeMs;
      }
      return 0.f;
    }
};

struct CurrentPhase {
    short phaseIndex;
    unsigned long timeInPhase;
    bool isRestrictionHit;
};

struct PhaseConditions {
    unsigned long timeInPhase;
};

struct Phases {
    short count;
    Phase *phases;

    CurrentPhase getCurrentPhase(PhaseConditions& phaseConditions) {
        short phase = 0;
        unsigned long accumulatedTime = 0;

        while (phase < count - 1 && phaseConditions.timeInPhase >= accumulatedTime + phases[phase].durationMs) {
            accumulatedTime += phases[phase].durationMs;
            phase += 1;
        }
        return CurrentPhase{phase, phaseConditions.timeInPhase - accumulatedTime};
    }
};

#endif
