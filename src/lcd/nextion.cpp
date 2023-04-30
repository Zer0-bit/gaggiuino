/* 09:32 15/03/2023 - change triggering comment */
#include "lcd.h"
#include "pindef.h"
#include "log.h"
#include <Arduino.h>

EasyNex myNex(USART_LCD);
volatile int lcdCurrentPageId;
volatile int lcdLastCurrentPageId;

void lcdInit(void) {
  myNex.begin(115200);
  while (myNex.readNumber("ack") != 100 )
  {
    LOG_VERBOSE("Connecting to Nextion LCD");
    delay(100);
  }
  myNex.writeStr("splash.build_version.txt", AUTO_VERSION);
  lcdCurrentPageId = myNex.currentPageId;
  lcdLastCurrentPageId = myNex.currentPageId;
}

void lcdListen(void) {
  myNex.NextionListen();
  lcdCurrentPageId = myNex.currentPageId;
}

void lcdWakeUp(void) {
  myNex.writeNum("sleep", 0);
}

void lcdUploadCfg(eepromValues_t &eepromCurrentValues) {
  myNex.writeNum("pIdx", eepromCurrentValues.idx);
  // PI
  myNex.writeNum("piState", eepromCurrentValues.preinfusionState);
  myNex.writeNum("piFlowState", eepromCurrentValues.preinfusionFlowState);

  if(eepromCurrentValues.preinfusionFlowState == 0) {
    myNex.writeNum("preinfusion.piTime.val", eepromCurrentValues.preinfusionSec);
    myNex.writeNum("preinfusion.piFlow.val", eepromCurrentValues.preinfusionPressureFlowTarget * 10.f);
    myNex.writeNum("preinfusion.piBar.val", eepromCurrentValues.preinfusionBar * 10.f);
  }
  else {
    myNex.writeNum("preinfusion.piTime.val", eepromCurrentValues.preinfusionFlowTime);
    myNex.writeNum("preinfusion.piFlow.val", eepromCurrentValues.preinfusionFlowVol * 10.f);
    myNex.writeNum("preinfusion.piBar.val", eepromCurrentValues.preinfusionFlowPressureTarget * 10.f);
  }
  myNex.writeNum("preinfusion.piPumped.val", eepromCurrentValues.preinfusionFilled);
  myNex.writeNum("piRPressure", eepromCurrentValues.preinfusionPressureAbove);
  myNex.writeNum("preinfusion.piAbove.val", eepromCurrentValues.preinfusionWeightAbove * 10.f);

  // SOAK
  myNex.writeNum("skState", eepromCurrentValues.soakState);

  if(eepromCurrentValues.preinfusionFlowState == 0)
    myNex.writeNum("soak.skTime.val", eepromCurrentValues.soakTimePressure);
  else
    myNex.writeNum("soak.skTime.val", eepromCurrentValues.soakTimeFlow);

  myNex.writeNum("soak.skBar.val", eepromCurrentValues.soakKeepPressure * 10.f);
  myNex.writeNum("soak.skFlow.val", eepromCurrentValues.soakKeepFlow * 10.f);
  myNex.writeNum("soak.skBelow.val", eepromCurrentValues.soakBelowPressure * 10.f);
  myNex.writeNum("soak.skAbv.val", eepromCurrentValues.soakAbovePressure * 10.f);
  myNex.writeNum("soak.skWAbv.val", eepromCurrentValues.soakAboveWeight * 10.f);
  // PI -> PF
  myNex.writeNum("soak.skRamp.val", eepromCurrentValues.preinfusionRamp);
  myNex.writeNum("skCrv", eepromCurrentValues.preinfusionRampSlope);
  // PROFILING
  myNex.writeNum("ppState", eepromCurrentValues.profilingState);
  myNex.writeNum("ppType", eepromCurrentValues.flowProfileState);

  if(eepromCurrentValues.flowProfileState == 0) {
    myNex.writeNum("profiles.pStart.val", eepromCurrentValues.pressureProfilingStart * 10.f);
    myNex.writeNum("profiles.pEnd.val", eepromCurrentValues.pressureProfilingFinish * 10.f);
    myNex.writeNum("profiles.pHold.val", eepromCurrentValues.pressureProfilingHold);
    myNex.writeNum("profiles.hLim.val", eepromCurrentValues.pressureProfilingHoldLimit * 10.f);
    myNex.writeNum("profiles.pSlope.val", eepromCurrentValues.pressureProfilingSlope);
    myNex.writeNum("profiles.pLim.val", eepromCurrentValues.pressureProfilingFlowRestriction * 10.f);
  } else {
    myNex.writeNum("profiles.pStart.val", eepromCurrentValues.flowProfileStart * 10.f);
    myNex.writeNum("profiles.pEnd.val", eepromCurrentValues.flowProfileEnd * 10.f);
    myNex.writeNum("profiles.pHold.val", eepromCurrentValues.flowProfileHold);
    myNex.writeNum("profiles.hLim.val", eepromCurrentValues.flowProfileHoldLimit * 10.f);
    myNex.writeNum("profiles.pSlope.val", eepromCurrentValues.flowProfileSlope);
    myNex.writeNum("profiles.pLim.val", eepromCurrentValues.flowProfilingPressureRestriction * 10.f);
  }
  myNex.writeNum("pfCrv", eepromCurrentValues.flowProfileSlopeShape);

  // More brew settings
  myNex.writeNum("homeOnBrewFinish", eepromCurrentValues.homeOnShotFinish);
  myNex.writeNum("brewSettings.btGoHome.val", eepromCurrentValues.homeOnShotFinish);

  myNex.writeNum("basketPrefill", eepromCurrentValues.basketPrefill);
  myNex.writeNum("brewSettings.btPrefill.val", eepromCurrentValues.basketPrefill);

  myNex.writeNum("deltaState", eepromCurrentValues.brewDeltaState);
  myNex.writeNum("brewSettings.btTempDelta.val", eepromCurrentValues.brewDeltaState);

  // System settings
  myNex.writeNum("moreTemp.setPoint.val", eepromCurrentValues.setpoint);
  myNex.writeNum("moreTemp.steamSetPoint.val", eepromCurrentValues.steamSetPoint);
  myNex.writeNum("moreTemp.offSet.val", eepromCurrentValues.offsetTemp);
  myNex.writeNum("moreTemp.hpwr.val", eepromCurrentValues.hpwr);
  myNex.writeNum("moreTemp.mDiv.val", eepromCurrentValues.mainDivider);
  myNex.writeNum("moreTemp.bDiv.val", eepromCurrentValues.brewDivider);

  myNex.writeNum("systemSleepTime", eepromCurrentValues.lcdSleep*60);
  myNex.writeNum("morePower.n1.val", eepromCurrentValues.lcdSleep);
  myNex.writeNum("morePower.lc1.val", eepromCurrentValues.scalesF1);
  myNex.writeNum("morePower.lc2.val", eepromCurrentValues.scalesF2);
  myNex.writeNum("morePower.pump_zero.val", eepromCurrentValues.pumpFlowAtZero * 10000.f);
  myNex.writeNum("warmupState", eepromCurrentValues.warmupState);

  // Dose settings
  myNex.writeNum("shotState", eepromCurrentValues.stopOnWeightState);
  myNex.writeNum("shotSettings.numDose.val", eepromCurrentValues.shotDose * 10.f);
  myNex.writeNum("shotPreset", eepromCurrentValues.shotPreset);
  myNex.writeNum("shotSettings.numDoseForced.val", eepromCurrentValues.shotStopOnCustomWeight * 10.f);
}

void uploadPageCfg(eepromValues_t &eepromCurrentValues) {
  // Updating only page specific elements as necessary to speed up things and avoid needless writes.
  switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)) {
    case SCREEN_MODES::SCREEN_brew_preinfusion:
      // PI
      myNex.writeNum("piState", eepromCurrentValues.preinfusionState);
      myNex.writeNum("piFlowState", eepromCurrentValues.preinfusionFlowState);

      if(eepromCurrentValues.preinfusionFlowState == 0) {
        myNex.writeNum("preinfusion.piTime.val", eepromCurrentValues.preinfusionSec);
        myNex.writeNum("preinfusion.piFlow.val", eepromCurrentValues.preinfusionPressureFlowTarget * 10.f);
        myNex.writeNum("preinfusion.piBar.val", eepromCurrentValues.preinfusionBar * 10.f);
      }
      else {
        myNex.writeNum("preinfusion.piTime.val", eepromCurrentValues.preinfusionFlowTime);
        myNex.writeNum("preinfusion.piFlow.val", eepromCurrentValues.preinfusionFlowVol * 10.f);
        myNex.writeNum("preinfusion.piBar.val", eepromCurrentValues.preinfusionFlowPressureTarget * 10.f);
      }
      myNex.writeNum("preinfusion.piPumped.val", eepromCurrentValues.preinfusionFilled);
      myNex.writeNum("piRPressure", eepromCurrentValues.preinfusionPressureAbove);
      myNex.writeNum("preinfusion.piAbove.val", eepromCurrentValues.preinfusionWeightAbove * 10.f);
      break;
    case SCREEN_MODES::SCREEN_brew_soak:
      myNex.writeNum("skState", eepromCurrentValues.soakState);

      if(eepromCurrentValues.preinfusionFlowState == 0)
        myNex.writeNum("soak.skTime.val", eepromCurrentValues.soakTimePressure);
      else
        myNex.writeNum("soak.skTime.val", eepromCurrentValues.soakTimeFlow);

      myNex.writeNum("soak.skBar.val", eepromCurrentValues.soakKeepPressure * 10.f);
      myNex.writeNum("soak.skFlow.val", eepromCurrentValues.soakKeepFlow * 10.f);
      myNex.writeNum("soak.skBelow.val", eepromCurrentValues.soakBelowPressure * 10.f);
      myNex.writeNum("soak.skAbv.val", eepromCurrentValues.soakAbovePressure * 10.f);
      myNex.writeNum("soak.skWAbv.val", eepromCurrentValues.soakAboveWeight * 10.f);
      // PI -> PF
      myNex.writeNum("soak.skRamp.val", eepromCurrentValues.preinfusionRamp);
      myNex.writeNum("skCrv", eepromCurrentValues.preinfusionRampSlope);
      break;
    case SCREEN_MODES::SCREEN_brew_profiling:
      // PROFILING
      myNex.writeNum("ppState", eepromCurrentValues.profilingState);
      myNex.writeNum("ppType", eepromCurrentValues.flowProfileState);

      if(eepromCurrentValues.flowProfileState == 0) {
        myNex.writeNum("profiles.pStart.val", eepromCurrentValues.pressureProfilingStart * 10.f);
        myNex.writeNum("profiles.pEnd.val", eepromCurrentValues.pressureProfilingFinish * 10.f);
        myNex.writeNum("profiles.pHold.val", eepromCurrentValues.pressureProfilingHold);
        myNex.writeNum("profiles.hLim.val", eepromCurrentValues.pressureProfilingHoldLimit * 10.f);
        myNex.writeNum("profiles.pSlope.val", eepromCurrentValues.pressureProfilingSlope);
        myNex.writeNum("profiles.pLim.val", eepromCurrentValues.pressureProfilingFlowRestriction * 10.f);
      } else {
        myNex.writeNum("profiles.pStart.val", eepromCurrentValues.flowProfileStart * 10.f);
        myNex.writeNum("profiles.pEnd.val", eepromCurrentValues.flowProfileEnd * 10.f);
        myNex.writeNum("profiles.pHold.val", eepromCurrentValues.flowProfileHold);
        myNex.writeNum("profiles.hLim.val", eepromCurrentValues.flowProfileHoldLimit * 10.f);
        myNex.writeNum("profiles.pSlope.val", eepromCurrentValues.flowProfileSlope);
        myNex.writeNum("profiles.pLim.val", eepromCurrentValues.flowProfilingPressureRestriction * 10.f);
      }
      myNex.writeNum("pfCrv", eepromCurrentValues.flowProfileSlopeShape);
      break;
    default:
      lcdUploadCfg(eepromCurrentValues);
      break;
  }
}

eepromValues_t lcdDownloadCfg(void) {
  eepromValues_t lcdCfg = {};
  // Profile ID
  lcdCfg.idx = myNex.readNumber("pIdx");
  // PI
  lcdCfg.preinfusionState = myNex.readNumber("piState");
  lcdCfg.preinfusionFlowState = myNex.readNumber("piFlowState");

  if(lcdCfg.preinfusionFlowState == 0) {
    lcdCfg.preinfusionSec = myNex.readNumber("preinfusion.piTime.val");
    lcdCfg.preinfusionPressureFlowTarget = myNex.readNumber("preinfusion.piFlow.val") / 10.f;
    lcdCfg.preinfusionBar = myNex.readNumber("preinfusion.piBar.val") / 10.f;
  }
  else {
    lcdCfg.preinfusionFlowTime = myNex.readNumber("preinfusion.piTime.val");
    lcdCfg.preinfusionFlowVol = myNex.readNumber("preinfusion.piFlow.val") / 10.f;
    lcdCfg.preinfusionFlowPressureTarget = myNex.readNumber("preinfusion.piBar.val") / 10.f;
  }
  lcdCfg.preinfusionFilled = myNex.readNumber("preinfusion.piPumped.val");
  lcdCfg.preinfusionPressureAbove = myNex.readNumber("piRPressure");
  lcdCfg.preinfusionWeightAbove = myNex.readNumber("preinfusion.piAbove.val") / 10.f;
  // SOAK
  lcdCfg.soakState = myNex.readNumber("skState");

  if(lcdCfg.preinfusionFlowState == 0)
    lcdCfg.soakTimePressure = myNex.readNumber("soak.skTime.val");
  else
    lcdCfg.soakTimeFlow = myNex.readNumber("soak.skTime.val");

  lcdCfg.soakKeepPressure = myNex.readNumber("soak.skBar.val") / 10.f;
  lcdCfg.soakKeepFlow = myNex.readNumber("soak.skFlow.val") / 10.f;
  lcdCfg.soakBelowPressure = myNex.readNumber("soak.skBelow.val") / 10.f;
  lcdCfg.soakAbovePressure = myNex.readNumber("soak.skAbv.val") / 10.f;
  lcdCfg.soakAboveWeight = myNex.readNumber("soak.skWAbv.val") / 10.f;
  // PI -> PF
  lcdCfg.preinfusionRamp = myNex.readNumber("soak.skRamp.val");
  lcdCfg.preinfusionRampSlope = myNex.readNumber("skCrv");
  // PROFILING
  lcdCfg.profilingState = myNex.readNumber("ppState");
  lcdCfg.flowProfileState = myNex.readNumber("ppType");

  if(lcdCfg.flowProfileState == 0) {
    lcdCfg.pressureProfilingStart = myNex.readNumber("profiles.pStart.val") / 10.f;
    lcdCfg.pressureProfilingFinish = myNex.readNumber("profiles.pEnd.val") / 10.f;
    lcdCfg.pressureProfilingHold = myNex.readNumber("profiles.pHold.val");
    lcdCfg.pressureProfilingHoldLimit = myNex.readNumber("profiles.hLim.val") / 10.f;
    lcdCfg.pressureProfilingSlope = myNex.readNumber("profiles.pSlope.val");
    lcdCfg.pressureProfilingFlowRestriction = myNex.readNumber("profiles.pLim.val") / 10.f;
  } else {
    lcdCfg.flowProfileStart = myNex.readNumber("profiles.pStart.val") / 10.f;
    lcdCfg.flowProfileEnd = myNex.readNumber("profiles.pEnd.val") / 10.f;
    lcdCfg.flowProfileHold = myNex.readNumber("profiles.pHold.val");
    lcdCfg.flowProfileHoldLimit = myNex.readNumber("profiles.hLim.val") / 10.f;
    lcdCfg.flowProfileSlope = myNex.readNumber("profiles.pSlope.val");
    lcdCfg.flowProfilingPressureRestriction = myNex.readNumber("profiles.pLim.val") / 10.f;
  }
  lcdCfg.flowProfileSlopeShape = myNex.readNumber("pfCrv");
  // More brew settings
  lcdCfg.homeOnShotFinish               = myNex.readNumber("homeOnBrewFinish");
  lcdCfg.basketPrefill                  = myNex.readNumber("basketPrefill");
  lcdCfg.brewDeltaState                 = myNex.readNumber("deltaState");
  // DOse settings
  lcdCfg.stopOnWeightState              = myNex.readNumber("shotState");
  lcdCfg.shotDose                       = myNex.readNumber("shotSettings.numDose.val") / 10.f;
  lcdCfg.shotStopOnCustomWeight         = myNex.readNumber("shotSettings.numDoseForced.val") / 10.f;
  lcdCfg.shotPreset                     = myNex.readNumber("shotPreset");
  // System settings
  lcdCfg.setpoint                       = myNex.readNumber("moreTemp.setPoint.val");
  lcdCfg.steamSetPoint                  = myNex.readNumber("moreTemp.steamSetPoint.val");
  lcdCfg.offsetTemp                     = myNex.readNumber("moreTemp.offSet.val");
  lcdCfg.hpwr                           = myNex.readNumber("moreTemp.hpwr.val");
  lcdCfg.mainDivider                    = myNex.readNumber("moreTemp.mDiv.val");
  lcdCfg.brewDivider                    = myNex.readNumber("moreTemp.bDiv.val");
  lcdCfg.lcdSleep                       = myNex.readNumber("systemSleepTime") / 60;
  lcdCfg.warmupState                    = myNex.readNumber("warmupState");
  lcdCfg.scalesF1                       = myNex.readNumber("morePower.lc1.val");
  lcdCfg.scalesF2                       = myNex.readNumber("morePower.lc2.val");
  lcdCfg.pumpFlowAtZero                 = myNex.readNumber("morePower.pump_zero.val") / 10000.f;

  return lcdCfg;
}

int lcdGetManualFlowVol(void) {
  return myNex.readNumber("h0.val");
}

int lcdGetHomeScreenScalesEnabled(void) {
  return myNex.readNumber("scalesEnabled");
}

int lcdGetSelectedOperationalMode(void) {
  return  myNex.readNumber("modeSelect");
}

int lcdGetDescaleCycle(void) {
  return myNex.readNumber("j0.val");
}

void lcdSetDescaleCycle(int cycle) {
  myNex.writeNum("j0.val", cycle);
}

void lcdSetPressure(float val) {
  myNex.writeNum("pressure.val", val);
}

void lcdSetUpTime(float val) {
  myNex.writeNum("systemUpTime", val);
}

void lcdSetTemperature(uint16_t val) {
  myNex.writeNum("currentTemp", val);
}

void lcdSetWeight(float val) {
  char tmp[6];
  int check = snprintf(tmp, sizeof(tmp), "%.1f", static_cast<double>(val));
  if (check > 0 && static_cast<unsigned int>(check) <= sizeof(tmp)) {
    strcat(tmp, "g");
    myNex.writeStr("weight.txt", tmp);
  }
}

void lcdSetFlow(int val) {
  myNex.writeNum("flow.val", val);
}

void lcdShowDebug(int val1, int val2) {
  myNex.writeNum("debug1",val1);
  myNex.writeNum("debug2",val2);
}

void lcdShowPopup(const char *msg) {
  static unsigned int timer;
  if(millis() > timer + 1000) {
    myNex.writeStr("popupMSG.t0.txt", msg);
    myNex.writeStr("page popupMSG");
    timer = millis();
  }
}

void lcdTargetState(int val) {
  myNex.writeNum("targetState", val);
}

void lcdBrewTimerStart(void) {
  myNex.writeNum("timerState", 1);
}

void lcdBrewTimerStop(void) {
  myNex.writeNum("timerState", 0);
}

void lcdSetBrewTimer(int seconds) {
  myNex.writeNum("activeBrewTime", seconds);
}

void lcdWarmupStateStop(void) {
  myNex.writeNum("warmupState", 0);
}

void trigger1(void) { lcdSaveSettingsTrigger(); }
void trigger2(void) { lcdScalesTareTrigger(); }
void trigger3(void) { lcdHomeScreenScalesTrigger(); }
void trigger4(void) { lcdBrewGraphScalesTareTrigger(); }
void trigger5(void) { lcdPumpPhaseShitfTrigger(); }
void trigger6(void) { lcdRefreshElementsTrigger(); }
void trigger7(void) { lcdqPSelctTrigger(); }
void trigger8(void) { lcdqPSaveTrigger(); }
