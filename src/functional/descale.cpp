#include "descale.h"

void deScale(eepromValues_t &runningCfg, SensorState &currentState) {
  static bool blink = true;
  static long timer = millis();
  static int currentCycleRead = 0;
  static int lastCycleRead = 5;
  static bool descaleFinished = false;

  if (brewState() && !descaleFinished) {
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
    if ((millis() - timer) > 1000) {
      lcdBrewTimerStop();
      lcdShowDescaleFinished();
      timer=millis();
    }
  } else {
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
  delay(200);
  openValve();
  delay(200);
  closeValve();
  delay(200);
  openValve();
  delay(200);
  closeValve();
  delay(200);
  openValve();
  setPumpOff();
}
