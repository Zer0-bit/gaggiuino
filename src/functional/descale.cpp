#include "../peripherals/internal_watchdog.h"
#include "descale.h"

DescalingState descalingState = DescalingState::IDLE;

short flushCounter = 0;
uint8_t counter = 0;
unsigned long descalingTimer = 0;
int descalingCycle = 0;

void deScale(eepromValues_t &runningCfg, SensorState &currentState) {
  switch (descalingState) {
    case DescalingState::IDLE:
      if (brewState()) {
        runningCfg.setpoint = 9;
        openValve();
        descalingState = DescalingState::DESCALING_PHASE1;
        descalingCycle = 0;
        descalingTimer = millis();
      }
      break;
    case DescalingState::DESCALING_PHASE1: // Slowly penetrating that scale
      brewState() ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpToRawValue(10);
      if (millis() - descalingTimer > DESCALE_PHASE1_EVERY) {
        lcdSetDescaleCycle(descalingCycle++);
        if (descalingCycle < 100) {
          descalingTimer = millis();
          descalingState = DescalingState::DESCALING_PHASE2;
        } else {
          descalingState = DescalingState::FINISHED;
        }
      }
      break;
    case DescalingState::DESCALING_PHASE2: // Softening the f outta that scale
      brewState() ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpOff();
      if (millis() - descalingTimer > DESCALE_PHASE2_EVERY) {
        descalingTimer = millis();
        lcdSetDescaleCycle(descalingCycle++);
        descalingState = DescalingState::DESCALING_PHASE3;
      }
      break;
    case DescalingState::DESCALING_PHASE3: // Fucking up that scale big time
      brewState() ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpToRawValue(30);
      if (millis() - descalingTimer > DESCALE_PHASE3_EVERY) {
        solenoidBeat();
        lcdSetDescaleCycle(descalingCycle++);
        if (descalingCycle < 100) {
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
      brewState() ? descalingState = DescalingState::FINISHED : descalingState = DescalingState::IDLE;
      if (millis() - descalingTimer > 1000) {
        lcdBrewTimerStop();
        lcdShowPopup("FINISHED");
        descalingTimer = millis();
      }
      break;
  }
  justDoCoffee(runningCfg, currentState, false, false);
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
  if (brewState()) {
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
    if ((flushCounter % 2)) {
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
