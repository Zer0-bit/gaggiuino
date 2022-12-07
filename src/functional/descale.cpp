#include "descale.h"

short flushCounter;

void deScale(eepromValues_t &runningCfg, SensorState &currentState) {
  static bool blink = true;
  static long timer = millis();
  static int currentCycleRead = 0;
  static int lastCycleRead = 5;
  static bool descaleFinished = false;

  if (brewState() && !descaleFinished) {
    openValve();
    if (currentCycleRead < lastCycleRead) { // descale in cycles of 5 then wait according to the below conditions
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        setPumpToRawValue(10);
        if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to min descale value for 10 sec
          if(currentCycleRead < 100) lcdSetDescaleCycle(currentCycleRead);
          else descaleFinished = true;
          blink = false;
          timer = millis();
        }
      } else {
        setPumpOff();
        if ((millis() - timer) > DESCALE_PHASE2_EVERY) { //nothing for 5 minutes
          currentCycleRead++;
          if(currentCycleRead < 100) lcdSetDescaleCycle(currentCycleRead);
          else descaleFinished = true;
          blink = true;
          timer = millis();
        }
      }
    } else {
      setPumpToRawValue(30);
      if (millis() - timer > DESCALE_PHASE3_EVERY) { //set dimmer power to max descale value for 20 sec
        solenoidBeat();
        blink = true;
        currentCycleRead++;
        lastCycleRead = currentCycleRead*3;
        if (lastCycleRead < 100) lcdSetDescaleCycle(currentCycleRead);
        else {
          lcdSetDescaleCycle(100);
          descaleFinished = true;
        }
        timer = millis();
      }
    }
  } else if (brewState() && descaleFinished == true){
    setPumpOff();
    closeValve();
    if ((millis() - timer) > 1000) {
      lcdBrewTimerStop();
      lcdShowPopup("FINISHED");
      timer=millis();
    }
  } else {
    setPumpOff();
    closeValve();
    currentCycleRead = 0;
    lastCycleRead = 10;
    descaleFinished = false;
    timer = millis();
  }
  //keeping it at temp
  justDoCoffee(runningCfg, currentState, false, false);
}

void solenoidBeat() {
  setPumpFullOn();
  closeValve();
  delay(1000);
  openValve();
  delay(200);
  closeValve();
  delay(1000);
  openValve();
  delay(200);
  closeValve();
  delay(1000);
  openValve();
  setPumpOff();
}

void backFlush(SensorState &currentState) {
  static unsigned long backflushTimer = millis();
  unsigned long elapsedTime = millis() - backflushTimer;
  if (brewState()) {
    if (flushCounter >= 11) {
      flushDeactivated();
    }
    else if (lcdCurrentPageId == 4 && elapsedTime <= 7000UL && currentState.smoothedPressure < 5.f) {
      flushActivated();
    } else {
      flushPhases();
    }
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
