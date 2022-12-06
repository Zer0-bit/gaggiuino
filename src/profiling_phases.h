#ifndef PROFILING_PHASES_H
#define PROFILING_PHASES_H

#include "utils.h"
#include "sensors_state.h"
#include "eeprom_data/eeprom_data.h"

enum PHASE_TYPE {
  PHASE_TYPE_FLOW,
  PHASE_TYPE_PRESSURE
};

struct StopConditions {
  long phaseDuration = -1;
  float weight = -1; //example: when weight >0 stop this phase)
  float pressureAbove = -1;
  float pressureBelow = -1;
  float waterVolume = -1;
  long shotTime = -1;

  bool isReached(SensorState& state, long timeInPhase, long timeInShot) {
    float flow = state.weight > 0.4f ? state.weightFlow : state.smoothedPumpFlow;
    float stopWeight = weight + flow / 2.f;

    return (weight > 0.f && state.shotWeight > stopWeight) ||
      (pressureAbove > 0 && state.smoothedPressure > pressureAbove) ||
      (pressureBelow > 0 && state.smoothedPressure < pressureBelow) ||
      (waterVolume > 0 && state.liquidPumped > waterVolume) ||
      (shotTime > 0 && timeInShot > shotTime) ||
      (phaseDuration >= 0 && timeInPhase >= phaseDuration) ;
  }
};

struct Phase {
    PHASE_TYPE type;
    float startValue;
    float endValue;
    float startRestriction;
    float endRestriction;
    StopConditions stopConditions;

    float getTarget(unsigned long timeInPhase) {
      return mapRange(timeInPhase, 0, stopConditions.phaseDuration, startValue, endValue, 1);
    }

    float getRestriction(unsigned long timeInPhase) {
      return mapRange(timeInPhase, 0, stopConditions.phaseDuration, startRestriction, endRestriction, 1);
    }

    bool isStopConditionReached(SensorState& currentState, unsigned long timeInPhase, unsigned long timeInShot) {
      return stopConditions.isReached(currentState, timeInPhase, timeInShot);
    }
};

class CurrentPhase {
private:
  int index;
  Phase &phase;
  unsigned long timeInPhase;

public:
  CurrentPhase(int index, Phase& phase, unsigned long timeInPhase)
  : index(index), phase{phase}, timeInPhase(timeInPhase) {}

  CurrentPhase(const CurrentPhase& currentPhase)
  : index(currentPhase.index), phase{currentPhase.phase}, timeInPhase(currentPhase.timeInPhase) {}

  Phase getPhase() {
    return phase;
  }

  PHASE_TYPE getType() {
    return phase.type;
  }

  int getIndex() {
    return index;
  }

  long getTimeInPhase() {
    return timeInPhase;
  }

  float getTarget() {
    return phase.getTarget(timeInPhase);
  }

  float getRestriction() {
    return phase.getRestriction(timeInPhase);
  }

  void update(int index, Phase& phase, unsigned long timeInPhase) {
    CurrentPhase::index = index;
    CurrentPhase::phase = phase;
    CurrentPhase::timeInPhase = timeInPhase;
  }
};

struct Phases {
  short count;
  Phase *phases;
};

class PhaseProfiler {
private:
  Phases& phases;
  short currentPhaseIdx = 0; // The index at which the profiler currently is.
  long phaseChangedTime = 0; // When did the profiler move to this currentPhaseIdx (in milliseconds since the shot started)
  CurrentPhase currentPhase = CurrentPhase(0, phases.phases[0], 0);

public:
  PhaseProfiler(Phases& phases): phases(phases) {}

  // Gets the profiling phase we should be in based on the timeInShot and the Sensors state
  CurrentPhase& getCurrentPhase(long timeInShot, SensorState& state) {
    short phaseIdx = currentPhaseIdx;
    unsigned long timeInPhase = timeInShot - phaseChangedTime;

    if (phaseIdx >= phases.count) {
      phaseIdx = phases.count - 1;
      currentPhase.update(phaseIdx, phases.phases[phaseIdx], timeInPhase);
      return currentPhase;
    }

    if (!phases.phases[phaseIdx].isStopConditionReached(state, timeInPhase, timeInShot)) {
      currentPhase.update(phaseIdx, phases.phases[phaseIdx], timeInPhase);
      return currentPhase;
    }

    Phase& phase = phases.phases[phaseIdx];
    long maxTimeAdvancement = (phase.stopConditions.phaseDuration > 0) ? phase.stopConditions.phaseDuration : timeInPhase;

    currentPhaseIdx += 1;
    phaseChangedTime += fmin(maxTimeAdvancement, timeInPhase);
    return getCurrentPhase(timeInShot, state);
  }

  bool isFinished() {
    return currentPhaseIdx >= phases.count;
  }

  void reset() {
    currentPhaseIdx = 0;
    phaseChangedTime = 0;
  }
};

#endif
