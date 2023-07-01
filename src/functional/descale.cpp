/* 09:32 15/03/2023 - change triggering comment */
#include "descale.h"
#include "just_do_coffee.h"
#include "../peripherals/internal_watchdog.h"
#include "../peripherals/esp_comms.h"

const int DESCALE_MAX_CYCLES = 100;

DescalingState descalingState = DescalingState::IDLE;

short flushCounter = 0;
uint8_t counter = 0;
unsigned long descalingTimer = 0;
int descalingCycle = 0;

void deScale(GaggiaSettings &gaggiaSettings, const SensorState &currentState, SystemState& systemState) {
  switch (descalingState) {
    case DescalingState::IDLE: // Waiting for fuckfest to begin
      if (currentState.brewSwitchState) {
        openValve();
        setSteamValveRelayOn();
        descalingState = DescalingState::DESCALING_PHASE1;
        descalingCycle = 0;
        descalingTimer = millis();
      }
      break;
    case DescalingState::DESCALING_PHASE1: // Slowly penetrating that scale
      currentState.brewSwitchState ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpToRawValue(10);
      if (millis() - descalingTimer > DESCALE_PHASE1_EVERY) {
        descalingCycle += 1;
        if (descalingCycle < DESCALE_MAX_CYCLES) {
          descalingTimer = millis();
          descalingState = DescalingState::DESCALING_PHASE2;
        } else {
          descalingState = DescalingState::FINISHED;
        }
      }
      break;
    case DescalingState::DESCALING_PHASE2: // Softening the f outta that scale
      currentState.brewSwitchState ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpOff();
      if (millis() - descalingTimer > DESCALE_PHASE2_EVERY) {
        descalingTimer = millis();
        descalingCycle += 1;
        descalingState = DescalingState::DESCALING_PHASE3;
      }
      break;
    case DescalingState::DESCALING_PHASE3: // Fucking up that scale big time
      currentState.brewSwitchState ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpToRawValue(30);
      if (millis() - descalingTimer > DESCALE_PHASE3_EVERY) {
        solenoidBeat();
        descalingCycle += 1;
        if (descalingCycle < DESCALE_MAX_CYCLES) {
          descalingTimer = millis();
          descalingState = DescalingState::DESCALING_PHASE1;
        } else {
          descalingState = DescalingState::FINISHED;
        }
      }
      break;
    case DescalingState::FINISHED: // Scale successufuly fucked
      setPumpOff();
      closeValve();
      setSteamValveRelayOff();
      currentState.brewSwitchState ? descalingState = DescalingState::FINISHED : descalingState = DescalingState::IDLE;
      if (millis() - descalingTimer > 1000) {
        // lcdBrewTimerStop(); TODO
        espCommsSendNotification(Notification::info("Descale finished!"));
        descalingTimer = millis();
      }
      break;
  }
  systemState.descaleProgress = descalingCycle / DESCALE_MAX_CYCLES;
  justDoCoffee(gaggiaSettings, currentState, 9.f, false);
}

void solenoidBeat() {
  setPumpFullOn();
  closeValve();
  delay(1000);
  watchdogReload();
  openValve();
  delay(200);
  closeValve();
  delay(1000);
  watchdogReload();
  openValve();
  delay(200);
  closeValve();
  delay(1000);
  watchdogReload();
  openValve();
  setPumpOff();
}

void backFlush(const SensorState &currentState) {
  static unsigned long backflushTimer = millis();
  unsigned long elapsedTime = millis() - backflushTimer;
  if (currentState.brewSwitchState) {
    if (flushCounter >= 11) {
      flushDeactivated();
      return;
    }
    else if (elapsedTime > 7000UL && currentState.smoothedPressure > 5.f) {
      flushPhases();
    } else flushActivated();
  } else {
    flushDeactivated();
    flushCounter = 0;
    backflushTimer = millis();
  }
}


void flushActivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      openValve();
  #endif
  setPumpFullOn();
}

void flushDeactivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      closeValve();
  #endif
  setPumpOff();
}

void flushPhases(void) {
  static long timer = millis();
  if (flushCounter <= 10) {
    if (flushCounter % 2) {
      if (millis() - timer >= 5000) {
        flushCounter++;
        timer = millis();
      }
      openValve();
      setPumpFullOn();
    } else {
      if (millis() - timer >= 5000) {
        flushCounter++;
        timer = millis();
      }
      closeValve();
      setPumpOff();
    }
  } else {
    flushDeactivated();
    timer = millis();
  }
}
