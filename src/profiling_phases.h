#ifndef PROFILING_PHASES_H
#define PROFILING_PHASES_H

#include "utils.h"

enum PHASE_TYPE {
  PHASE_TYPE_FLOW,
  PHASE_TYPE_PRESSURE,
 };

struct Phase {
    PHASE_TYPE type;
    float startValue;
    float endValue;
    float startRestriction;
    float endRestriction;
    unsigned int durationMs;

    float getTarget(unsigned long timeInPhase) {
      return mapRange(timeInPhase, 0, durationMs, startValue, endValue, 1);
    }

    float getRestriction(unsigned long timeInPhase) {
      return mapRange(timeInPhase, 0, durationMs, startRestriction, endRestriction, 1);
    }
};

struct CurrentPhase {
    short phaseIndex;
    unsigned long timeInPhase;
};

struct Phases {
    short count;
    Phase *phases;

    CurrentPhase getCurrentPhase(unsigned long timeInPhase) {
        short phase = 0;
        unsigned long accumulatedTime = 0;

        while (phase < count - 1 && timeInPhase >= accumulatedTime + (phases[phase].durationMs)) {
            accumulatedTime += phases[phase].durationMs;
            phase += 1;
        }
        return CurrentPhase{phase, timeInPhase - accumulatedTime};
    }
};

#endif
