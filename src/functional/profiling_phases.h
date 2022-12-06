#ifndef PROFILING_PHASES_H
#define PROFILING_PHASES_H

#include "utils.h"
#include "sensors_state.h"
#include "../eeprom_data/eeprom_data.h"

enum PHASE_TYPE {
  PHASE_TYPE_FLOW,
  PHASE_TYPE_PRESSURE
};

struct ShotSnapshot {
  long timeInShot;
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
  float flowInPhase = -1;

  bool isReached(SensorState& state, long timeInShot, ShotSnapshot stateAtPhaseStart);
};

struct GlobalStopConditions {
  long time = -1;
  float weight = -1;
  float waterPumped = -1;

  bool isReached(SensorState& state, long timeInShot);
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

  float getTarget(unsigned long timeInPhase);
  float getRestriction();
  bool isStopConditionReached(SensorState& currentState, unsigned long timeInShot, ShotSnapshot stateAtPhaseStart);
};

class CurrentPhase {
private:
  int index;
  Phase& phase;
  unsigned long timeInPhase;

public:
  CurrentPhase(int index, Phase& phase, unsigned long timeInPhase);
  CurrentPhase(const CurrentPhase& currentPhase);

  Phase getPhase();
  PHASE_TYPE getType();
  int getIndex();
  long getTimeInPhase();
  float getTarget();
  float getRestriction();
  void update(int index, Phase& phase, unsigned long timeInPhase);
};

struct Phases {
  short count;
  Phase* phases;
};

class PhaseProfiler {
private:
  Phases& phases;
  short currentPhaseIdx = 0; // The index at which the profiler currently is.
  ShotSnapshot phaseChangedSnapshot = ShotSnapshot{0, 0, 0, 0, 0, 0}; // State when the profiler move to this currentPhaseIdx
  CurrentPhase currentPhase = CurrentPhase(0, phases.phases[0], 0);
  GlobalStopConditions globalStopConditions;

public:
  PhaseProfiler(Phases& phases);
  // Gets the profiling phase we should be in based on the timeInShot and the Sensors state
  void updatePhase(long timeInShot, SensorState& state);
  CurrentPhase& getCurrentPhase();
  bool isFinished();
  void reset();
  void updateGlobalStopConditions(float weight, long time = -1, float waterVolume = -1);
};

#endif
