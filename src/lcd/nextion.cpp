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

void lcdUploadProfile(eepromValues_t &eepromCurrentValues) {
  // Highlight the active profile
  myNex.writeNum("pId", eepromCurrentValues.activeProfile + 1  /* 1-offset in nextion */);
  // PI
  myNex.writeNum("piState", ACTIVE_PROFILE(eepromCurrentValues).preinfusionState);
  myNex.writeNum("piFlowState", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState);

  if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0) {
    myNex.writeNum("pi.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionSec);
    myNex.writeNum("pi.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureFlowTarget * 10.f);
    myNex.writeNum("pi.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionBar * 10.f);
  }
  else {
    myNex.writeNum("pi.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowTime);
    myNex.writeNum("pi.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowVol * 10.f);
    myNex.writeNum("pi.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowPressureTarget * 10.f);
  }
  myNex.writeNum("pi.piPumped.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFilled);
  myNex.writeNum("piRPressure", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureAbove);
  myNex.writeNum("pi.piAbove.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionWeightAbove * 10.f);

  // SOAK
  myNex.writeNum("skState", ACTIVE_PROFILE(eepromCurrentValues).soakState);

  if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0)
    myNex.writeNum("sk.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimePressure);
  else
    myNex.writeNum("sk.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimeFlow);

  myNex.writeNum("sk.skBar.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepPressure * 10.f);
  myNex.writeNum("sk.skFlow.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepFlow * 10.f);
  myNex.writeNum("sk.skBelow.val", ACTIVE_PROFILE(eepromCurrentValues).soakBelowPressure * 10.f);
  myNex.writeNum("sk.skAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAbovePressure * 10.f);
  myNex.writeNum("sk.skWAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAboveWeight * 10.f);
  // PI -> PF
  myNex.writeNum("sk.skRamp.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRamp);
  myNex.writeNum("skCrv", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRampSlope);
  // PROFILING
  myNex.writeNum("ppState", ACTIVE_PROFILE(eepromCurrentValues).profilingState);
  myNex.writeNum("ppType", ACTIVE_PROFILE(eepromCurrentValues).flowProfileState);

  if(ACTIVE_PROFILE(eepromCurrentValues).flowProfileState == 0) {
    myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingStart * 10.f);
    myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFinish * 10.f);
    myNex.writeNum("pf.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHold);
    myNex.writeNum("pf.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHoldLimit * 10.f);
    myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingSlope);
    myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingSlopeShape);
    myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFlowRestriction * 10.f);
  } else {
    myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileStart * 10.f);
    myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileEnd * 10.f);
    myNex.writeNum("pf.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHold);
    myNex.writeNum("pf.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHoldLimit * 10.f);
    myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlope);
    myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlopeShape);
    myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfilingPressureRestriction * 10.f);
  }
}

// This is never called again after boot
void lcdUploadCfg(eepromValues_t &eepromCurrentValues) {
  // bool profileType = false;
  // Highlight the active profile
  myNex.writeNum("pId", eepromCurrentValues.activeProfile + 1); /* 1-offset in nextion */

  // Profile names for all buttons
  myNex.writeStr("home.qPf1.txt", eepromCurrentValues.profiles[0].name);
  myNex.writeStr("home.qPf2.txt", eepromCurrentValues.profiles[1].name);
  myNex.writeStr("home.qPf3.txt", eepromCurrentValues.profiles[2].name);
  myNex.writeStr("home.qPf4.txt", eepromCurrentValues.profiles[3].name);
  myNex.writeStr("home.qPf5.txt", eepromCurrentValues.profiles[4].name);

  // More brew settings
  myNex.writeNum("homeOnBrewFinish", eepromCurrentValues.homeOnShotFinish);
  myNex.writeNum("bS.btGoHome.val", eepromCurrentValues.homeOnShotFinish);

  myNex.writeNum("basketPrefill", eepromCurrentValues.basketPrefill);
  myNex.writeNum("bS.btPrefill.val", eepromCurrentValues.basketPrefill);

  myNex.writeNum("deltaState", eepromCurrentValues.brewDeltaState);
  myNex.writeNum("bS.btTempDelta.val", eepromCurrentValues.brewDeltaState);

  // System settings
  myNex.writeNum("sT.setPoint.val", eepromCurrentValues.setpoint);
  myNex.writeNum("sT.steamSetPoint.val", eepromCurrentValues.steamSetPoint);
  myNex.writeNum("sT.offSet.val", eepromCurrentValues.offsetTemp);
  myNex.writeNum("sT.hpwr.val", eepromCurrentValues.hpwr);
  myNex.writeNum("sT.mDiv.val", eepromCurrentValues.mainDivider);
  myNex.writeNum("sT.bDiv.val", eepromCurrentValues.brewDivider);

  myNex.writeNum("systemSleepTime", eepromCurrentValues.lcdSleep*60);
  myNex.writeNum("sP.n1.val", eepromCurrentValues.lcdSleep);
  myNex.writeNum("sP.lc1.val", eepromCurrentValues.scalesF1);
  myNex.writeNum("sP.lc2.val", eepromCurrentValues.scalesF2);
  myNex.writeNum("sP.pump_zero.val", eepromCurrentValues.pumpFlowAtZero * 10000.f);
  myNex.writeNum("warmupState", eepromCurrentValues.warmupState);

  // Dose settings
  myNex.writeNum("shotState", eepromCurrentValues.stopOnWeightState);
  myNex.writeNum("dS.numDose.val", eepromCurrentValues.shotDose * 10.f);
  myNex.writeNum("shotPreset", eepromCurrentValues.shotPreset);
  myNex.writeNum("dS.numDoseForced.val", eepromCurrentValues.shotStopOnCustomWeight * 10.f);

  lcdUploadProfile(eepromCurrentValues);
}

void uploadPageCfg(eepromValues_t &eepromCurrentValues) {
  // Updating only page specific elements as necessary to speed up things and avoid needless writes.
  switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)) {
    case SCREEN_MODES::SCREEN_brew_preinfusion:
      // PI
      myNex.writeNum("piState", ACTIVE_PROFILE(eepromCurrentValues).preinfusionState);
      myNex.writeNum("piFlowState", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState);

      if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0) {
        myNex.writeNum("pi.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionSec);
        myNex.writeNum("pi.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureFlowTarget * 10.f);
        myNex.writeNum("pi.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionBar * 10.f);
      }
      else {
        myNex.writeNum("pi.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowTime);
        myNex.writeNum("pi.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowVol * 10.f);
        myNex.writeNum("pi.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowPressureTarget * 10.f);
      }
      myNex.writeNum("pi.piPumped.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFilled);
      myNex.writeNum("piRPressure", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureAbove);
      myNex.writeNum("pi.piAbove.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionWeightAbove * 10.f);
      break;
    case SCREEN_MODES::SCREEN_brew_soak:
      myNex.writeNum("skState", ACTIVE_PROFILE(eepromCurrentValues).soakState);

      if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0)
        myNex.writeNum("sk.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimePressure);
      else
        myNex.writeNum("sk.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimeFlow);

      myNex.writeNum("sk.skBar.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepPressure * 10.f);
      myNex.writeNum("sk.skFlow.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepFlow * 10.f);
      myNex.writeNum("sk.skBelow.val", ACTIVE_PROFILE(eepromCurrentValues).soakBelowPressure * 10.f);
      myNex.writeNum("sk.skAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAbovePressure * 10.f);
      myNex.writeNum("sk.skWAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAboveWeight * 10.f);
      // PI -> PF
      myNex.writeNum("sk.skRamp.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRamp);
      myNex.writeNum("skCrv", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRampSlope);
      break;
    case SCREEN_MODES::SCREEN_brew_profiling:
      // PROFILING
      myNex.writeNum("ppState", ACTIVE_PROFILE(eepromCurrentValues).profilingState);
      myNex.writeNum("ppType", ACTIVE_PROFILE(eepromCurrentValues).flowProfileState);

      if(ACTIVE_PROFILE(eepromCurrentValues).flowProfileState == 0) {
        myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingStart * 10.f);
        myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFinish * 10.f);
        myNex.writeNum("pf.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHold);
        myNex.writeNum("pf.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHoldLimit * 10.f);
        myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingSlope);
        myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingSlopeShape);
        myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFlowRestriction * 10.f);
      } else {
        myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileStart * 10.f);
        myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileEnd * 10.f);
        myNex.writeNum("pf.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHold);
        myNex.writeNum("pf.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHoldLimit * 10.f);
        myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlope);
        myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlopeShape);
        myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfilingPressureRestriction * 10.f);
      }
      break;
    default:
      lcdUploadCfg(eepromCurrentValues);
      break;
  }
}

/**
* This will only download profile params for a single profile. It could be set
* to any index of the array, as we basically load runningCfg with what's in the
* screen and let the profiler phases follow ACTIVE_PROFILE() of that.
*
* To keep things consistent, however, we're setting the params in the "correct" index.
*/
eepromValues_t lcdDownloadCfg(bool toSave) {
  eepromValues_t lcdCfg = eepromGetCurrentValues();
  lcdCfg.activeProfile = lcdGetSelectedProfile();
  // We only want to download values when actually switching a page with switchable vals.
  switch (lcdCurrentPageId) {
    case 4:
    case 5:
    case 6:
    case 9:
    case 12:
    case 13:
    return lcdCfg;
  }

  if (toSave) { // Only read this string when saving a profile
    char buttonElemId[16]; // 15+1 bytes in the btn element name.
    snprintf(buttonElemId, 16, "home.qPf%d.txt", lcdCfg.activeProfile + 1);
    snprintf(ACTIVE_PROFILE(lcdCfg).name, sizeof(ACTIVE_PROFILE(lcdCfg).name), "%.16s", myNex.readStr(buttonElemId).c_str());
  }

  if (toSave || lcdCurrentPageId == 1 || lcdLastCurrentPageId == 1) { // PI
    ACTIVE_PROFILE(lcdCfg).preinfusionState = myNex.readNumber("piState");
    ACTIVE_PROFILE(lcdCfg).preinfusionFlowState = myNex.readNumber("piFlowState");

    if(ACTIVE_PROFILE(lcdCfg).preinfusionFlowState == 0) {
      ACTIVE_PROFILE(lcdCfg).preinfusionSec = myNex.readNumber("pi.piTime.val");
      ACTIVE_PROFILE(lcdCfg).preinfusionPressureFlowTarget = myNex.readNumber("pi.piFlow.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).preinfusionBar = myNex.readNumber("pi.piBar.val") / 10.f;
    }
    else {
      ACTIVE_PROFILE(lcdCfg).preinfusionFlowTime = myNex.readNumber("pi.piTime.val");
      ACTIVE_PROFILE(lcdCfg).preinfusionFlowVol = myNex.readNumber("pi.piFlow.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).preinfusionFlowPressureTarget = myNex.readNumber("pi.piBar.val") / 10.f;
    }
    ACTIVE_PROFILE(lcdCfg).preinfusionFilled = myNex.readNumber("pi.piPumped.val");
    ACTIVE_PROFILE(lcdCfg).preinfusionPressureAbove = myNex.readNumber("piRPressure");
    ACTIVE_PROFILE(lcdCfg).preinfusionWeightAbove = myNex.readNumber("pi.piAbove.val") / 10.f;
  }

  if (toSave || lcdCurrentPageId == 2 || lcdLastCurrentPageId == 2) { // SOAK
    ACTIVE_PROFILE(lcdCfg).soakState = myNex.readNumber("skState");

    if(ACTIVE_PROFILE(lcdCfg).preinfusionFlowState == 0)
      ACTIVE_PROFILE(lcdCfg).soakTimePressure = myNex.readNumber("sk.skTime.val");
    else
      ACTIVE_PROFILE(lcdCfg).soakTimeFlow = myNex.readNumber("sk.skTime.val");

    ACTIVE_PROFILE(lcdCfg).soakKeepPressure = myNex.readNumber("sk.skBar.val") / 10.f;
    ACTIVE_PROFILE(lcdCfg).soakKeepFlow = myNex.readNumber("sk.skFlow.val") / 10.f;
    ACTIVE_PROFILE(lcdCfg).soakBelowPressure = myNex.readNumber("sk.skBelow.val") / 10.f;
    ACTIVE_PROFILE(lcdCfg).soakAbovePressure = myNex.readNumber("sk.skAbv.val") / 10.f;
    ACTIVE_PROFILE(lcdCfg).soakAboveWeight = myNex.readNumber("sk.skWAbv.val") / 10.f;
    // PI -> PF
    ACTIVE_PROFILE(lcdCfg).preinfusionRamp = myNex.readNumber("sk.skRamp.val");
    ACTIVE_PROFILE(lcdCfg).preinfusionRampSlope = myNex.readNumber("skCrv");
  }

  if (toSave || lcdCurrentPageId == 3 || lcdLastCurrentPageId == 3) {// PROFILING
    ACTIVE_PROFILE(lcdCfg).profilingState = myNex.readNumber("ppState");
    ACTIVE_PROFILE(lcdCfg).flowProfileState = myNex.readNumber("ppType");

    if(ACTIVE_PROFILE(lcdCfg).flowProfileState == 0) {
      ACTIVE_PROFILE(lcdCfg).pressureProfilingStart = myNex.readNumber("pf.pStart.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).pressureProfilingFinish = myNex.readNumber("pf.pEnd.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).pressureProfilingHold = myNex.readNumber("pf.pHold.val");
      ACTIVE_PROFILE(lcdCfg).pressureProfilingHoldLimit = myNex.readNumber("pf.hLim.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).pressureProfilingSlope = myNex.readNumber("pf.pSlope.val");
      ACTIVE_PROFILE(lcdCfg).pressureProfilingSlopeShape = myNex.readNumber("pfCrv");
      ACTIVE_PROFILE(lcdCfg).pressureProfilingFlowRestriction = myNex.readNumber("pf.pLim.val") / 10.f;
    } else {
      ACTIVE_PROFILE(lcdCfg).flowProfileStart = myNex.readNumber("pf.pStart.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).flowProfileEnd = myNex.readNumber("pf.pEnd.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).flowProfileHold = myNex.readNumber("pf.pHold.val");
      ACTIVE_PROFILE(lcdCfg).flowProfileHoldLimit = myNex.readNumber("pf.hLim.val") / 10.f;
      ACTIVE_PROFILE(lcdCfg).flowProfileSlope = myNex.readNumber("pf.pSlope.val");
      ACTIVE_PROFILE(lcdCfg).flowProfileSlopeShape = myNex.readNumber("pfCrv");
      ACTIVE_PROFILE(lcdCfg).flowProfilingPressureRestriction = myNex.readNumber("pf.pLim.val") / 10.f;
    }
  }

  if (toSave || lcdCurrentPageId == 10 || lcdLastCurrentPageId == 10) {// More brew settings
    lcdCfg.homeOnShotFinish               = myNex.readNumber("homeOnBrewFinish");
    lcdCfg.basketPrefill                  = myNex.readNumber("basketPrefill");
    lcdCfg.brewDeltaState                 = myNex.readNumber("deltaState");
  }

  if (toSave || lcdCurrentPageId == 11 || lcdLastCurrentPageId == 11) {// DOse settings
    lcdCfg.stopOnWeightState              = myNex.readNumber("shotState");
    lcdCfg.shotDose                       = myNex.readNumber("dS.numDose.val") / 10.f;
    lcdCfg.shotStopOnCustomWeight         = myNex.readNumber("dS.numDoseForced.val") / 10.f;
    lcdCfg.shotPreset                     = myNex.readNumber("shotPreset");
  }

  if (toSave || lcdCurrentPageId == 7 || lcdLastCurrentPageId == 7) {// System settings
    lcdCfg.setpoint                       = myNex.readNumber("sT.setPoint.val");
    lcdCfg.steamSetPoint                  = myNex.readNumber("sT.steamSetPoint.val");
    lcdCfg.offsetTemp                     = myNex.readNumber("sT.offSet.val");
    lcdCfg.hpwr                           = myNex.readNumber("sT.hpwr.val");
    lcdCfg.mainDivider                    = myNex.readNumber("sT.mDiv.val");
    lcdCfg.brewDivider                    = myNex.readNumber("sT.bDiv.val");
  }

  if (toSave || lcdCurrentPageId == 8 || lcdLastCurrentPageId == 8) {// System settings
    lcdCfg.lcdSleep                       = myNex.readNumber("systemSleepTime") / 60;
    lcdCfg.warmupState                    = myNex.readNumber("warmupState");
    lcdCfg.scalesF1                       = myNex.readNumber("sP.lc1.val");
    lcdCfg.scalesF2                       = myNex.readNumber("sP.lc2.val");
    lcdCfg.pumpFlowAtZero                 = myNex.readNumber("sP.pump_zero.val") / 10000.f;
  }

  return lcdCfg;
}

uint8_t lcdGetSelectedProfile(void) {
  uint32_t pId = myNex.readNumber("pId");
  if (pId < 1 || pId > 5) {
    pId = myNex.readNumber("pId");
    if (pId < 1 || pId > 5) lcdShowPopup((String("getProfile rekt: ") + pId).c_str());
  }
  return (uint8_t)(pId - 1); /* 1-offset in nextion */
}

int lcdGetManualFlowVol(void) {
  return myNex.readNumber("h0.val");
}

int lcdGetHomeScreenScalesEnabled(void) {
  return myNex.readNumber("scEn.val");
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
void trigger6(void) { lcdRefreshElementsTrigger(); }
void trigger7(void) { lcdQuickProfileSwitch(); }
void trigger8(void) { lcdSaveProfileTrigger(); }
