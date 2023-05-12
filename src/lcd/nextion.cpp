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
  myNex.writeNum("pIdx", eepromCurrentValues.activeProfile + 1  /* 1-offset in nextion */);
  // PI
  myNex.writeNum("piState", ACTIVE_PROFILE(eepromCurrentValues).preinfusionState);
  myNex.writeNum("piFlowState", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState);

  if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0) {
    myNex.writeNum("preinfusion.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionSec);
    myNex.writeNum("preinfusion.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureFlowTarget * 10.f);
    myNex.writeNum("preinfusion.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionBar * 10.f);
  }
  else {
    myNex.writeNum("preinfusion.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowTime);
    myNex.writeNum("preinfusion.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowVol * 10.f);
    myNex.writeNum("preinfusion.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowPressureTarget * 10.f);
  }
  myNex.writeNum("preinfusion.piPumped.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFilled);
  myNex.writeNum("piRPressure", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureAbove);
  myNex.writeNum("preinfusion.piAbove.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionWeightAbove * 10.f);

  // SOAK
  myNex.writeNum("skState", ACTIVE_PROFILE(eepromCurrentValues).soakState);

  if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0)
    myNex.writeNum("soak.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimePressure);
  else
    myNex.writeNum("soak.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimeFlow);

  myNex.writeNum("soak.skBar.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepPressure * 10.f);
  myNex.writeNum("soak.skFlow.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepFlow * 10.f);
  myNex.writeNum("soak.skBelow.val", ACTIVE_PROFILE(eepromCurrentValues).soakBelowPressure * 10.f);
  myNex.writeNum("soak.skAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAbovePressure * 10.f);
  myNex.writeNum("soak.skWAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAboveWeight * 10.f);
  // PI -> PF
  myNex.writeNum("soak.skRamp.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRamp);
  myNex.writeNum("skCrv", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRampSlope);
  // PROFILING
  myNex.writeNum("ppState", ACTIVE_PROFILE(eepromCurrentValues).profilingState);
  myNex.writeNum("ppType", ACTIVE_PROFILE(eepromCurrentValues).flowProfileState);

  if(ACTIVE_PROFILE(eepromCurrentValues).flowProfileState == 0) {
    myNex.writeNum("profiles.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingStart * 10.f);
    myNex.writeNum("profiles.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFinish * 10.f);
    myNex.writeNum("profiles.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHold);
    myNex.writeNum("profiles.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHoldLimit * 10.f);
    myNex.writeNum("profiles.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingSlope);
    myNex.writeNum("profiles.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFlowRestriction * 10.f);
  } else {
    myNex.writeNum("profiles.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileStart * 10.f);
    myNex.writeNum("profiles.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileEnd * 10.f);
    myNex.writeNum("profiles.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHold);
    myNex.writeNum("profiles.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHoldLimit * 10.f);
    myNex.writeNum("profiles.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlope);
    myNex.writeNum("profiles.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfilingPressureRestriction * 10.f);
  }
  myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlopeShape);
}

// This is never called again after boot
void lcdUploadCfg(eepromValues_t &eepromCurrentValues) {
  // bool profileType = false;
  // Highlight the active profile
  myNex.writeNum("pIdx", eepromCurrentValues.activeProfile + 1  /* 1-offset in nextion */);

  // Profile names for all buttons
  myNex.writeStr("home.qPf1.txt", eepromCurrentValues.profiles[0].name);
  myNex.writeStr("home.qPf2.txt", eepromCurrentValues.profiles[1].name);
  myNex.writeStr("home.qPf3.txt", eepromCurrentValues.profiles[2].name);
  myNex.writeStr("home.qPf4.txt", eepromCurrentValues.profiles[3].name);
  myNex.writeStr("home.qPf5.txt", eepromCurrentValues.profiles[4].name);

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
        myNex.writeNum("preinfusion.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionSec);
        myNex.writeNum("preinfusion.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureFlowTarget * 10.f);
        myNex.writeNum("preinfusion.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionBar * 10.f);
      }
      else {
        myNex.writeNum("preinfusion.piTime.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowTime);
        myNex.writeNum("preinfusion.piFlow.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowVol * 10.f);
        myNex.writeNum("preinfusion.piBar.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowPressureTarget * 10.f);
      }
      myNex.writeNum("preinfusion.piPumped.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionFilled);
      myNex.writeNum("piRPressure", ACTIVE_PROFILE(eepromCurrentValues).preinfusionPressureAbove);
      myNex.writeNum("preinfusion.piAbove.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionWeightAbove * 10.f);
      break;
    case SCREEN_MODES::SCREEN_brew_soak:
      myNex.writeNum("skState", ACTIVE_PROFILE(eepromCurrentValues).soakState);

      if(ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState == 0)
        myNex.writeNum("soak.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimePressure);
      else
        myNex.writeNum("soak.skTime.val", ACTIVE_PROFILE(eepromCurrentValues).soakTimeFlow);

      myNex.writeNum("soak.skBar.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepPressure * 10.f);
      myNex.writeNum("soak.skFlow.val", ACTIVE_PROFILE(eepromCurrentValues).soakKeepFlow * 10.f);
      myNex.writeNum("soak.skBelow.val", ACTIVE_PROFILE(eepromCurrentValues).soakBelowPressure * 10.f);
      myNex.writeNum("soak.skAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAbovePressure * 10.f);
      myNex.writeNum("soak.skWAbv.val", ACTIVE_PROFILE(eepromCurrentValues).soakAboveWeight * 10.f);
      // PI -> PF
      myNex.writeNum("soak.skRamp.val", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRamp);
      myNex.writeNum("skCrv", ACTIVE_PROFILE(eepromCurrentValues).preinfusionRampSlope);
      break;
    case SCREEN_MODES::SCREEN_brew_profiling:
      // PROFILING
      myNex.writeNum("ppState", ACTIVE_PROFILE(eepromCurrentValues).profilingState);
      myNex.writeNum("ppType", ACTIVE_PROFILE(eepromCurrentValues).flowProfileState);

      if(ACTIVE_PROFILE(eepromCurrentValues).flowProfileState == 0) {
        myNex.writeNum("profiles.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingStart * 10.f);
        myNex.writeNum("profiles.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFinish * 10.f);
        myNex.writeNum("profiles.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHold);
        myNex.writeNum("profiles.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingHoldLimit * 10.f);
        myNex.writeNum("profiles.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingSlope);
        myNex.writeNum("profiles.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).pressureProfilingFlowRestriction * 10.f);
      } else {
        myNex.writeNum("profiles.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileStart * 10.f);
        myNex.writeNum("profiles.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileEnd * 10.f);
        myNex.writeNum("profiles.pHold.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHold);
        myNex.writeNum("profiles.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileHoldLimit * 10.f);
        myNex.writeNum("profiles.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlope);
        myNex.writeNum("profiles.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).flowProfilingPressureRestriction * 10.f);
      }
      myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).flowProfileSlopeShape);
      break;
    default:
      lcdUploadCfg(eepromCurrentValues);
      break;
  }
}
void lcdFetchProfileName(eepromValues_t::profile_t &profile, uint8_t index /* 0-offset */) {
  char buttonElemId[16];
  snprintf(buttonElemId, 16, "home.qPf%d.txt", index + 1);
  snprintf(profile.name, sizeof(profile.name), "%.16s", myNex.readStr(buttonElemId).c_str());
}

void lcdFetchPreinfusion(eepromValues_t::profile_t &profile) {
  profile.preinfusionState = myNex.readNumber("piState");
  profile.preinfusionFlowState = lcdGetPreinfusionFlowState();

  if(profile.preinfusionFlowState == 0) {
    profile.preinfusionSec = myNex.readNumber("preinfusion.piTime.val");
    profile.preinfusionPressureFlowTarget = myNex.readNumber("preinfusion.piFlow.val") / 10.f;
    profile.preinfusionBar = myNex.readNumber("preinfusion.piBar.val") / 10.f;
  }
  else {
    profile.preinfusionFlowTime = myNex.readNumber("preinfusion.piTime.val");
    profile.preinfusionFlowVol = myNex.readNumber("preinfusion.piFlow.val") / 10.f;
    profile.preinfusionFlowPressureTarget = myNex.readNumber("preinfusion.piBar.val") / 10.f;
  }
  profile.preinfusionFilled = myNex.readNumber("preinfusion.piPumped.val");
  profile.preinfusionPressureAbove = myNex.readNumber("piRPressure");
  profile.preinfusionWeightAbove = myNex.readNumber("preinfusion.piAbove.val") / 10.f;
}

void lcdFetchSoak(eepromValues_t::profile_t &profile) {
  // SOAK
  profile.soakState = myNex.readNumber("skState");

  if(profile.preinfusionFlowState == 0)
    profile.soakTimePressure = myNex.readNumber("soak.skTime.val");
  else
    profile.soakTimeFlow = myNex.readNumber("soak.skTime.val");

  profile.soakKeepPressure = myNex.readNumber("soak.skBar.val") / 10.f;
  profile.soakKeepFlow = myNex.readNumber("soak.skFlow.val") / 10.f;
  profile.soakBelowPressure = myNex.readNumber("soak.skBelow.val") / 10.f;
  profile.soakAbovePressure = myNex.readNumber("soak.skAbv.val") / 10.f;
  profile.soakAboveWeight = myNex.readNumber("soak.skWAbv.val") / 10.f;
  // PI -> PF
  profile.preinfusionRamp = myNex.readNumber("soak.skRamp.val");
  profile.preinfusionRampSlope = myNex.readNumber("skCrv");
}

void lcdFetchBrewProfile(eepromValues_t::profile_t &profile) {
  // PROFILING
  profile.profilingState = myNex.readNumber("ppState");
  profile.flowProfileState = lcdGetProfileFlowState();

  if(profile.flowProfileState == 0) {
    profile.pressureProfilingStart = myNex.readNumber("profiles.pStart.val") / 10.f;
    profile.pressureProfilingFinish = myNex.readNumber("profiles.pEnd.val") / 10.f;
    profile.pressureProfilingHold = myNex.readNumber("profiles.pHold.val");
    profile.pressureProfilingHoldLimit = myNex.readNumber("profiles.hLim.val") / 10.f;
    profile.pressureProfilingSlope = myNex.readNumber("profiles.pSlope.val");
    profile.pressureProfilingSlopeShape = myNex.readNumber("pfCrv");
    profile.pressureProfilingFlowRestriction = myNex.readNumber("profiles.pLim.val") / 10.f;
  } else {
    profile.flowProfileStart = myNex.readNumber("profiles.pStart.val") / 10.f;
    profile.flowProfileEnd = myNex.readNumber("profiles.pEnd.val") / 10.f;
    profile.flowProfileHold = myNex.readNumber("profiles.pHold.val");
    profile.flowProfileHoldLimit = myNex.readNumber("profiles.hLim.val") / 10.f;
    profile.flowProfileSlope = myNex.readNumber("profiles.pSlope.val");
    profile.flowProfileSlopeShape = myNex.readNumber("pfCrv");
    profile.flowProfilingPressureRestriction = myNex.readNumber("profiles.pLim.val") / 10.f;
  }
}

void lcdFetchBrewSettings(eepromValues_t &settings) {
  // More brew settings
  settings.homeOnShotFinish               = myNex.readNumber("homeOnBrewFinish");
  settings.basketPrefill                  = myNex.readNumber("basketPrefill");
  settings.brewDeltaState                 = myNex.readNumber("deltaState");
}

// TODO: move these to inside the profile
void lcdFetchDoseSettings(eepromValues_t &settings) {
  // DOse settings
  settings.stopOnWeightState              = myNex.readNumber("shotState");
  settings.shotDose                       = myNex.readNumber("shotSettings.numDose.val") / 10.f;
  settings.shotStopOnCustomWeight         = myNex.readNumber("shotSettings.numDoseForced.val") / 10.f;
  settings.shotPreset                     = myNex.readNumber("shotPreset");
}

void lcdFetchTemp(eepromValues_t &settings) {
  settings.setpoint                       = myNex.readNumber("moreTemp.setPoint.val");
  settings.steamSetPoint                  = myNex.readNumber("moreTemp.steamSetPoint.val");
  settings.offsetTemp                     = myNex.readNumber("moreTemp.offSet.val");
  settings.hpwr                           = myNex.readNumber("moreTemp.hpwr.val");
  settings.mainDivider                    = myNex.readNumber("moreTemp.mDiv.val");
  settings.brewDivider                    = myNex.readNumber("moreTemp.bDiv.val");
 }

void lcdFetchSystem(eepromValues_t &settings) {
  // System settings
  settings.lcdSleep                       = myNex.readNumber("systemSleepTime") / 60;
  settings.warmupState                    = myNex.readNumber("warmupState");
  settings.scalesF1                       = myNex.readNumber("morePower.lc1.val");
  settings.scalesF2                       = myNex.readNumber("morePower.lc2.val");
  settings.pumpFlowAtZero                 = myNex.readNumber("morePower.pump_zero.val") / 10000.f;
}

int lcdGetSelectedProfile(void) {
  int selected = myNex.readNumber("pIdx");
  if (selected < 1 || selected > 5) lcdShowPopup((String("getProfile rekt: ") + selected).c_str());
  return selected - 1 /* 1-offset in nextion */;
}

bool lcdGetPreinfusionFlowState(void) {
  return myNex.readNumber("piFlowState");
}

bool lcdGetProfileFlowState(void) {
  return myNex.readNumber("ppType");
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
void trigger6(void) { lcdRefreshElementsTrigger(); }
void trigger7(void) { lcdQuickProfileSwitch(); }
void trigger8(void) { lcdSaveProfileTrigger(); }
