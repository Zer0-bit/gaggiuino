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
      if (runningCfg.flowProfileState) {
        return mapRange(currentState.pressure, 0, runningCfg.preinfusionFlowPressureTarget, runningCfg.preinfusionFlowTime, 0, 1);
      }
      else {
        return mapRange(currentState.pressure, 0, runningCfg.preinfusionBar ,runningCfg.preinfusionFlowTime, 0, 1);
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

    CurrentPhase getCurrentPhase(PhaseConditions& stopConditions) {
        short phase = 0;
        unsigned long accumulatedTime = 0;
        
        while (phase < count - 1 && stopConditions.timeInPhase >= accumulatedTime + phases[phase].durationMs) {
            accumulatedTime += phases[phase].durationMs;
            phase += 1;
        }
        return CurrentPhase{phase, stopConditions.timeInPhase - accumulatedTime};
    }
};

#endif
