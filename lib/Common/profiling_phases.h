/* 09:32 15/03/2023 - change triggering comment */
#ifndef PROFILING_PHASES_H
#define PROFILING_PHASES_H

#include "utils.h"
#include "sensors_state.h"
#include <vector>
#include <string>

enum class PhaseType {
  FLOW = 0,
  PRESSURE = 1
};

struct PhaseStopConditions {
  uint32_t time = 0;
  float pressureAbove = 0;
  float pressureBelow = 0;
  float flowAbove = 0;
  float flowBelow = 0;
  float weight = 0; //example: when pushed weight >0 stop this phase)
  float waterPumpedInPhase = 0;

  bool isReached(SensorState& state, uint32_t timeInShot, ShotSnapshot stateAtPhaseStart) const;
};

struct Transition {
  float start;
  float end;
  TransitionCurve curve;
  uint32_t time;

  Transition() : Transition(0) {}
  Transition(float targetValue, TransitionCurve curve = TransitionCurve::INSTANT, uint32_t time = 0) : Transition(0, targetValue, curve, time) {}
  Transition(float start, float end, TransitionCurve curve = TransitionCurve::LINEAR, uint32_t time = 0) : start(start), end(end), curve(curve), time(time) {
    if (end <= 0) end = start;
    if (end <= 0) time = 0;
  }

  bool isInstant() {
    return curve == TransitionCurve::INSTANT || time == 0;
  }
};

struct Phase {
  std::string name = "";
  PhaseType type;
  Transition target;
  float restriction = 0;
  PhaseStopConditions stopConditions;
  bool skip = false;
  float waterTemperature = 0;

  float getTarget(uint32_t timeInPhase, const ShotSnapshot& shotSnapshotAtStart) const;
  float getRestriction() const;
  bool isStopConditionReached(SensorState& currentState, uint32_t timeInShot, ShotSnapshot stateAtPhaseStart) const;
};

struct GlobalStopConditions {
  uint32_t time = 0;
  float weight = 0;
  float waterPumped = 0;

  bool isReached(const SensorState& state, uint32_t timeInShot) const;
};

struct BrewRecipe {
  float coffeeIn = 0;
  float coffeeOut = 0;
  float ratio = 0;

  float getCoffeeOut() const { return coffeeOut != 0 ? coffeeOut : coffeeIn * ratio; };
};

struct Profile {
  std::string name;
  std::vector<Phase> phases;
  GlobalStopConditions globalStopConditions;
  float waterTemperature = 0;
  BrewRecipe recipe;

  size_t phaseCount() const {
    return phases.size();
  }

  void addPhase(Phase phase) {
    phases.push_back(phase);
  }

  void insertPhase(Phase phase, size_t index) {
    phases.insert(phases.begin() + index, phase);
  }

  void clear() {
    phases.clear();
  }
};

#endif
