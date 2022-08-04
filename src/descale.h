#ifndef DESCALE_H
#define DESCALE_H

#include "gaggiuino.h"
#include "peripherals/pump.h"
#include "just_do_coffee.h"

#define DESCALE_PHASE1_EVERY    500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY    5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY    120000 // long pause for scale softening

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

static void deScale(void) {
  static bool blink = true;
  static long timer = millis();
  static int currentCycleRead = lcdGetDescaleCycle();
  static int lastCycleRead = 10;
  static bool descaleFinished = false;
  if (brewState() && !descaleFinished) {
    if (currentCycleRead < lastCycleRead) { // descale in cycles of 5 then wait according to the below conditions
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        setPumpToRawValue(15);
        if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to min descale value for 10 sec
          if (currentCycleRead >=100) descaleFinished = true;
          blink = false;
          currentCycleRead = lcdGetDescaleCycle();
          timer = millis();
        }
      } else {
        setPumpToRawValue(30);
        if (millis() - timer > DESCALE_PHASE2_EVERY) { //set dimmer power to max descale value for 20 sec
          blink = true;
          currentCycleRead++;
          if (currentCycleRead<100) lcdSetDescaleCycle(currentCycleRead);
          timer = millis();
        }
      }
    } else {
      setPumpOff();
      if ((millis() - timer) > DESCALE_PHASE3_EVERY) { //nothing for 5 minutes
        if (currentCycleRead*2 < 100) lcdSetDescaleCycle(currentCycleRead*3);
        else {
          lcdSetDescaleCycle(100);
          descaleFinished = true;
        }
        lastCycleRead = currentCycleRead*2;
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
  justDoCoffee();
}
#endif
