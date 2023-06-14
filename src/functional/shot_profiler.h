#ifndef SHOT_PROFILER_H
#define SHOT_PROFILER_H

#include "profiling_phases.h"

class CurrentPhase {
private:
  int index;
  const Phase* phase;
  uint32_t timeInPhase;
  const ShotSnapshot* shotSnapshotAtStart;

public:
  CurrentPhase(int index, const Phase& phase, uint32_t timeInPhase, const ShotSnapshot& shotSnapshotAtStart);
  CurrentPhase(const CurrentPhase& currentPhase);

  const Phase* getPhase();
  PhaseType getType();
  int getIndex();
  uint32_t getTimeInPhase();
  float getTarget();
  float getRestriction();
  void update(int index, const Phase& phase, uint32_t timeInPhase);
};

class PhaseProfiler {
private:
  const Profile* profile;
  size_t currentPhaseIdx = 0; // The index at which the profiler currently is.
  ShotSnapshot phaseChangedSnapshot = ShotSnapshot{ 0, 0, 0, 0, 0, 0 }; // State when the profiler move to this currentPhaseIdx
  CurrentPhase currentPhase = CurrentPhase(0, profile->phases[0], 0, phaseChangedSnapshot);

public:
  PhaseProfiler(const Profile& profile);
  // Gets the profiling phase we should be in based on the timeInShot and the Sensors state
  void setProfile(const Profile& profile) { this->profile = &profile; };
  void updatePhase(uint32_t timeInShot, SensorState& state);
  CurrentPhase& getCurrentPhase();
  bool isFinished();
  void reset();
};

// Helper functions

ShotSnapshot buildShotSnapshot(uint32_t timeInShot, const SensorState& state, CurrentPhase& phase);

#endif
