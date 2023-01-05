#ifndef PROFILING_PHASES_H
#define PROFILING_PHASES_H

#include "utils.h"
#include "sensors_state.h"

enum PHASE_TYPE {
  PHASE_TYPE_FLOW,
  PHASE_TYPE_PRESSURE
};

struct ShotSnapshot {
  uint32_t timeInShot;
  float pressure;
  float flow;
  float temperature;
  float shotWeight;
  float waterPumped;
};

struct PhaseStopConditions {
  long time = -1;
  float pressureAbove = -1;
  float pressureBelow = -1;
  float flowAbove = -1;
  float flowBelow = -1;
  float weight = -1; //example: when pushed weight >0 stop this phase)
  float waterPumpedInPhase = -1;

  bool isReached(SensorState& state, long timeInShot, ShotSnapshot stateAtPhaseStart) const;
};


struct Transition {
  float start;
  float end;
  TransitionCurve curve;
  long time;

  Transition(): start(-1), end(-1), curve(INSTANT), time(0) {}
  Transition(float start, float end, TransitionCurve curve = LINEAR, long time = 0): start(start), end(end), curve(curve), time(time) {}
  Transition(float value):  start(value), end(value), curve(INSTANT), time(0) {}
};

struct Phase {
  PHASE_TYPE type;
  Transition target;
  float restriction;
  PhaseStopConditions stopConditions;

  float getTarget(uint32_t timeInPhase) const;
  float getRestriction() const;
  bool isStopConditionReached(SensorState& currentState, uint32_t timeInShot, ShotSnapshot stateAtPhaseStart) const;
};

struct GlobalStopConditions {
  long time = -1;
  float weight = -1;
  float waterPumped = -1;

  bool isReached(SensorState& state, long timeInShot);
};

struct Profile {
  short count;
  Phase* phases;
  GlobalStopConditions globalStopConditions;
};

class CurrentPhase {
private:
  int index;
  const Phase* phase;
  unsigned long timeInPhase;

public:
  CurrentPhase(int index, const Phase& phase, uint32_t timeInPhase);
  CurrentPhase(const CurrentPhase& currentPhase);

  Phase getPhase();
  PHASE_TYPE getType();
  int getIndex();
  long getTimeInPhase();
  float getTarget();
  float getRestriction();
  void update(int index, const Phase& phase, uint32_t timeInPhase);
};

class PhaseProfiler {
private:
  Profile& profile;
  short currentPhaseIdx = 0; // The index at which the profiler currently is.
  ShotSnapshot phaseChangedSnapshot = ShotSnapshot{0, 0, 0, 0, 0, 0}; // State when the profiler move to this currentPhaseIdx
  CurrentPhase currentPhase = CurrentPhase(0, profile.phases[0], 0);

public:
  PhaseProfiler(Profile& profile);
  // Gets the profiling phase we should be in based on the timeInShot and the Sensors state
  void updatePhase(uint32_t timeInShot, SensorState& state);
  CurrentPhase& getCurrentPhase();
  bool isFinished();
  void reset();
};

#endif
