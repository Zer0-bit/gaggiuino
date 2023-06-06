/* 09:32 15/03/2023 - change triggering comment */
#include "lcd.h"
#include "pindef.h"
#include "log.h"
#include <Arduino.h>
#include "../peripherals/led.h"

EasyNex myNex(USART_LCD);
volatile NextionPage lcdCurrentPageId;
volatile NextionPage lcdLastCurrentPageId;
LED ledCtrl;

void lcdInit(void) {
  myNex.begin(115200);
  while (!lcdCheckSerialInit("\x88\xFF\xFF\xFF", 4)) {
    LOG_VERBOSE("Connecting to Nextion LCD...");
    delay(5);
  }
  myNex.writeStr("splash.build_version.txt", AUTO_VERSION);
  lcdCurrentPageId = static_cast<NextionPage>(myNex.currentPageId);
  lcdLastCurrentPageId = static_cast<NextionPage>(myNex.currentPageId);
}

bool lcdCheckSerialInit(const char* expectedOutput, size_t expectedLen) {
  size_t receivedLen = 0;

  while (receivedLen < expectedLen) {
    int receivedByte = myNex.readByte();
    if (receivedByte != -1) {
      if (receivedByte == expectedOutput[receivedLen]) {
        receivedLen++;
      } else {
        // Reset receivedLen if the received byte doesn't match
        receivedLen = 0;
      }
    }
  }

  // Serial output matches expected output
  return true;
}


void lcdListen(void) {
  myNex.NextionListen();
  lcdCurrentPageId = static_cast<NextionPage>(myNex.currentPageId);
}

void lcdWakeUp(void) {
  myNex.writeNum("sleep", 0);
}

void lcdUploadProfile(eepromValues_t &eepromCurrentValues) {
  // Highlight the active profile
  myNex.writeNum("pId", eepromCurrentValues.activeProfile + 1  /* 1-offset in nextion */);
  String buttonElemId = String("home.qPf") + (eepromCurrentValues.activeProfile + 1) + ".txt";
  myNex.writeStr(buttonElemId, ACTIVE_PROFILE(eepromCurrentValues).name);

  // Temp
  myNex.writeNum("sT.setPoint.val", ACTIVE_PROFILE(eepromCurrentValues).setpoint);
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
  // Adnvanced transition profile
  myNex.writeNum("paState", ACTIVE_PROFILE(eepromCurrentValues).tpState);
  myNex.writeNum("paType", ACTIVE_PROFILE(eepromCurrentValues).tpType);
  if(ACTIVE_PROFILE(eepromCurrentValues).tpType == 0) {
    myNex.writeNum("tp.tStart.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingStart * 10.f);
    myNex.writeNum("tp.tEnd.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingFinish * 10.f);
    myNex.writeNum("tp.tHold.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingHold);
    myNex.writeNum("tp.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingHoldLimit * 10.f);
    myNex.writeNum("tp.tSlope.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingSlope);
    myNex.writeNum("paCrv", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingSlopeShape);
    myNex.writeNum("tp.tLim.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingFlowRestriction * 10.f);
  } else {
    myNex.writeNum("tp.tStart.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileStart * 10.f);
    myNex.writeNum("tp.tEnd.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileEnd * 10.f);
    myNex.writeNum("tp.tHold.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileHold);
    myNex.writeNum("tp.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileHoldLimit * 10.f);
    myNex.writeNum("tp.tSlope.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileSlope);
    myNex.writeNum("paCrv", ACTIVE_PROFILE(eepromCurrentValues).tfProfileSlopeShape);
    myNex.writeNum("tp.tLim.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfilingPressureRestriction * 10.f);
  }
  // Main profile
  myNex.writeNum("ppState", ACTIVE_PROFILE(eepromCurrentValues).profilingState);
  myNex.writeNum("ppType", ACTIVE_PROFILE(eepromCurrentValues).mfProfileState);
  if(ACTIVE_PROFILE(eepromCurrentValues).mfProfileState == 0) {
    myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingStart * 10.f);
    myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingFinish * 10.f);
    myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingSlope);
    myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingSlopeShape);
    myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingFlowRestriction * 10.f);
  } else {
    myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfileStart * 10.f);
    myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfileEnd * 10.f);
    myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfileSlope);
    myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).mfProfileSlopeShape);
    myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfilingPressureRestriction * 10.f);
  }
  // Dose settings
  myNex.writeNum("shotState", ACTIVE_PROFILE(eepromCurrentValues).stopOnWeightState);
  myNex.writeNum("dS.numDose.val", ACTIVE_PROFILE(eepromCurrentValues).shotDose * 10.f);
  myNex.writeNum("shotPreset", ACTIVE_PROFILE(eepromCurrentValues).shotPreset);
  myNex.writeNum("dS.numDoseForced.val", ACTIVE_PROFILE(eepromCurrentValues).shotStopOnCustomWeight * 10.f);
}

// This is never called again after boot
void lcdUploadCfg(eepromValues_t &eepromCurrentValues) {
  // bool profileType = false;

  // Profile names for all buttons
  myNex.writeStr("home.qPf1.txt", eepromCurrentValues.profiles[0].name);
  myNex.writeStr("home.qPf2.txt", eepromCurrentValues.profiles[1].name);
  myNex.writeStr("home.qPf3.txt", eepromCurrentValues.profiles[2].name);
  myNex.writeStr("home.qPf4.txt", eepromCurrentValues.profiles[3].name);
  myNex.writeStr("home.qPf5.txt", eepromCurrentValues.profiles[4].name);

  // More brew settings
  myNex.writeNum("bckHome", eepromCurrentValues.homeOnShotFinish);
  myNex.writeNum("basketPrefill", eepromCurrentValues.basketPrefill);
  myNex.writeNum("deltaState", eepromCurrentValues.brewDeltaState);

  // System settings
  myNex.writeNum("sT.steamSetPoint.val", eepromCurrentValues.steamSetPoint);
  myNex.writeNum("sT.offSet.val", eepromCurrentValues.offsetTemp);
  myNex.writeNum("sT.hpwr.val", eepromCurrentValues.hpwr);
  myNex.writeNum("sT.mDiv.val", eepromCurrentValues.mainDivider);
  myNex.writeNum("sT.bDiv.val", eepromCurrentValues.brewDivider);

  myNex.writeNum("sP.n1.val", eepromCurrentValues.lcdSleep);
  myNex.writeNum("sP.lc1.val", eepromCurrentValues.scalesF1);
  myNex.writeNum("sP.lc2.val", eepromCurrentValues.scalesF2);
  myNex.writeNum("sP.pump_zero.val", eepromCurrentValues.pumpFlowAtZero * 10000.f);
  myNex.writeNum("warmupState", eepromCurrentValues.warmupState);

  lcdUploadProfile(eepromCurrentValues);
}

void uploadPageCfg(eepromValues_t &eepromCurrentValues) {
  // Updating only page specific elements as necessary to speed up things and avoid needless writes.
  switch (lcdCurrentPageId) {
    case NextionPage::BrewPreinfusion:
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
    case NextionPage::BrewSoak:
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
    case NextionPage::BrewProfiling:
      // PROFILING
      myNex.writeNum("ppState", ACTIVE_PROFILE(eepromCurrentValues).profilingState);
      myNex.writeNum("ppType", ACTIVE_PROFILE(eepromCurrentValues).mfProfileState);

      if(ACTIVE_PROFILE(eepromCurrentValues).mfProfileState == 0) {
        myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingStart * 10.f);
        myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingFinish * 10.f);
        myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingSlope);
        myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingSlopeShape);
        myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).mpProfilingFlowRestriction * 10.f);
      } else {
        myNex.writeNum("pf.pStart.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfileStart * 10.f);
        myNex.writeNum("pf.pEnd.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfileEnd * 10.f);
        myNex.writeNum("pf.pSlope.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfileSlope);
        myNex.writeNum("pfCrv", ACTIVE_PROFILE(eepromCurrentValues).mfProfileSlopeShape);
        myNex.writeNum("pf.pLim.val", ACTIVE_PROFILE(eepromCurrentValues).mfProfilingPressureRestriction * 10.f);
      }
      break;
    case NextionPage::BrewTransitionProfile:
      myNex.writeNum("paState", ACTIVE_PROFILE(eepromCurrentValues).tpState);
      myNex.writeNum("paType", ACTIVE_PROFILE(eepromCurrentValues).tpType);
      // Adnvanced transition profile
      if(ACTIVE_PROFILE(eepromCurrentValues).tpType == 0) {
        myNex.writeNum("tp.tStart.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingStart * 10.f);
        myNex.writeNum("tp.tEnd.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingFinish * 10.f);
        myNex.writeNum("tp.tHold.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingHold);
        myNex.writeNum("tp.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingHoldLimit * 10.f);
        myNex.writeNum("tp.tSlope.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingSlope);
        myNex.writeNum("paCrv", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingSlopeShape);
        myNex.writeNum("tp.tLim.val", ACTIVE_PROFILE(eepromCurrentValues).tpProfilingFlowRestriction * 10.f);
      } else {
        myNex.writeNum("tp.tStart.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileStart * 10.f);
        myNex.writeNum("tp.tEnd.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileEnd * 10.f);
        myNex.writeNum("tp.tHold.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileHold);
        myNex.writeNum("tp.hLim.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileHoldLimit * 10.f);
        myNex.writeNum("tp.tSlope.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfileSlope);
        myNex.writeNum("paCrv", ACTIVE_PROFILE(eepromCurrentValues).tfProfileSlopeShape);
        myNex.writeNum("tp.tLim.val", ACTIVE_PROFILE(eepromCurrentValues).tfProfilingPressureRestriction * 10.f);
      }
      break;
    default:
      lcdUploadCfg(eepromCurrentValues);
      break;
  }
}

void lcdFetchProfileName(eepromValues_t::profile_t &profile, uint8_t index /* 0-offset */) {
  String buttonElemId = String("home.qPf") + (index + 1) + ".txt";
  snprintf(profile.name, sizeof(profile.name), "%s", myNex.readStr(buttonElemId).c_str());
}

void lcdFetchPreinfusion(eepromValues_t::profile_t &profile) {
  profile.preinfusionState = myNex.readNumber("piState");
  profile.preinfusionFlowState = lcdGetPreinfusionFlowState();

  if(profile.preinfusionFlowState == 0) {
    profile.preinfusionSec = myNex.readNumber("pi.piTime.val");
    profile.preinfusionPressureFlowTarget = myNex.readNumber("pi.piFlow.val") / 10.f;
    profile.preinfusionBar = myNex.readNumber("pi.piBar.val") / 10.f;
  }
  else {
    profile.preinfusionFlowTime = myNex.readNumber("pi.piTime.val");
    profile.preinfusionFlowVol = myNex.readNumber("pi.piFlow.val") / 10.f;
    profile.preinfusionFlowPressureTarget = myNex.readNumber("pi.piBar.val") / 10.f;
  }
  profile.preinfusionFilled = myNex.readNumber("pi.piPumped.val");
  profile.preinfusionPressureAbove = myNex.readNumber("piRPressure");
  profile.preinfusionWeightAbove = myNex.readNumber("pi.piAbove.val") / 10.f;
}

void lcdFetchSoak(eepromValues_t::profile_t &profile) {
  // SOAK
  profile.soakState = myNex.readNumber("skState");

  if(profile.preinfusionFlowState == 0)
    profile.soakTimePressure = myNex.readNumber("sk.skTime.val");
  else
    profile.soakTimeFlow = myNex.readNumber("sk.skTime.val");

  profile.soakKeepPressure = myNex.readNumber("sk.skBar.val") / 10.f;
  profile.soakKeepFlow = myNex.readNumber("sk.skFlow.val") / 10.f;
  profile.soakBelowPressure = myNex.readNumber("sk.skBelow.val") / 10.f;
  profile.soakAbovePressure = myNex.readNumber("sk.skAbv.val") / 10.f;
  profile.soakAboveWeight = myNex.readNumber("sk.skWAbv.val") / 10.f;
  // PI -> PF
  profile.preinfusionRamp = myNex.readNumber("sk.skRamp.val");
  profile.preinfusionRampSlope = myNex.readNumber("skCrv");
}

void lcdFetchBrewProfile(eepromValues_t::profile_t &profile) {
  // PROFILING
  profile.profilingState = myNex.readNumber("ppState");
  profile.mfProfileState = lcdGetProfileFlowState();

  if(profile.mfProfileState == 0) {
    profile.mpProfilingStart = myNex.readNumber("pf.pStart.val") / 10.f;
    profile.mpProfilingFinish = myNex.readNumber("pf.pEnd.val") / 10.f;
    profile.mpProfilingSlope = myNex.readNumber("pf.pSlope.val");
    profile.mpProfilingSlopeShape = myNex.readNumber("pfCrv");
    profile.mpProfilingFlowRestriction = myNex.readNumber("pf.pLim.val") / 10.f;
  } else {
    profile.mfProfileStart = myNex.readNumber("pf.pStart.val") / 10.f;
    profile.mfProfileEnd = myNex.readNumber("pf.pEnd.val") / 10.f;
    profile.mfProfileSlope = myNex.readNumber("pf.pSlope.val");
    profile.mfProfileSlopeShape = myNex.readNumber("pfCrv");
    profile.mfProfilingPressureRestriction = myNex.readNumber("pf.pLim.val") / 10.f;
  }
}

void lcdFetchTransitionProfile(eepromValues_t::profile_t &profile) {
  profile.tpState = myNex.readNumber("paState");
  profile.tpType = lcdGetTransitionFlowState();

  if(profile.tpType == 0) {
    profile.tpProfilingStart = myNex.readNumber("tp.tStart.val") / 10.f;
    profile.tpProfilingFinish = myNex.readNumber("tp.tEnd.val") / 10.f;
    profile.tpProfilingHold = myNex.readNumber("tp.tHold.val");
    profile.tpProfilingHoldLimit = myNex.readNumber("tp.hLim.val") / 10.f;
    profile.tpProfilingSlope = myNex.readNumber("tp.tSlope.val");
    profile.tpProfilingSlopeShape = myNex.readNumber("paCrv");
    profile.tpProfilingFlowRestriction = myNex.readNumber("tp.tLim.val") / 10.f;
  } else {
    profile.tfProfileStart = myNex.readNumber("tp.tStart.val") / 10.f;
    profile.tfProfileEnd = myNex.readNumber("tp.tEnd.val") / 10.f;
    profile.tfProfileHold = myNex.readNumber("tp.tHold.val");
    profile.tfProfileHoldLimit = myNex.readNumber("tp.tLim.val") / 10.f;
    profile.tfProfileSlope = myNex.readNumber("tp.tSlope.val");
    profile.tfProfileSlopeShape = myNex.readNumber("paCrv");
    profile.tfProfilingPressureRestriction = myNex.readNumber("tp.tLim.val") / 10.f;
  }
}

void lcdFetchDoseSettings(eepromValues_t::profile_t &profile) {
  // DOse settings
  profile.stopOnWeightState              = myNex.readNumber("shotState");
  profile.shotDose                       = myNex.readNumber("dS.numDose.val") / 10.f;
  profile.shotStopOnCustomWeight         = myNex.readNumber("dS.numDoseForced.val") / 10.f;
  profile.shotPreset                     = myNex.readNumber("shotPreset");
}

void lcdFetchTemp(eepromValues_t::profile_t &profile) {
  profile.setpoint       = myNex.readNumber("sT.setPoint.val");
}

/**
* Overwrites the entire profile in the index corresponding to
* the currently selected profile on the screen.
*/
void lcdFetchCurrentProfile(eepromValues_t & settings) {
  // Target save to the currently selected profile on screen (can be different from runningCfg on long press)
  settings.activeProfile = lcdGetSelectedProfile();
  eepromValues_t::profile_t *profile = &settings.profiles[settings.activeProfile];

  lcdFetchProfileName(*profile, settings.activeProfile);
  lcdFetchPreinfusion(*profile);
  lcdFetchSoak(*profile);
  lcdFetchBrewProfile(*profile);
  lcdFetchTransitionProfile(*profile);
  lcdFetchDoseSettings(*profile);
  lcdFetchTemp(*profile);
}

void lcdFetchBrewSettings(eepromValues_t &settings) {
  // More brew settings
  settings.homeOnShotFinish               = myNex.readNumber("homeOnBrewFinish");
  settings.basketPrefill                  = myNex.readNumber("basketPrefill");
  settings.brewDeltaState                 = myNex.readNumber("deltaState");
}

void lcdFetchBoiler(eepromValues_t &settings) {
  settings.steamSetPoint                  = myNex.readNumber("sT.steamSetPoint.val");
  settings.offsetTemp                     = myNex.readNumber("sT.offSet.val");
  settings.hpwr                           = myNex.readNumber("sT.hpwr.val");
  settings.mainDivider                    = myNex.readNumber("sT.mDiv.val");
  settings.brewDivider                    = myNex.readNumber("sT.bDiv.val");
}

void lcdFetchSystem(eepromValues_t &settings) {
  // System settings
  settings.lcdSleep                       = myNex.readNumber("sP.n1.val"); // nextion sleep var
  settings.warmupState                    = myNex.readNumber("warmupState");
  settings.scalesF1                       = myNex.readNumber("sP.lc1.val");
  settings.scalesF2                       = myNex.readNumber("sP.lc2.val");
  settings.pumpFlowAtZero                 = myNex.readNumber("sP.pump_zero.val") / 10000.f;
}

void lcdFetchPage(eepromValues_t &settings, NextionPage page, int targetProfile) {
  switch (page) {
    case NextionPage::BrewMore:
      lcdFetchBrewSettings(settings);
      break;
    case NextionPage::BrewPreinfusion:
      lcdFetchPreinfusion(settings.profiles[targetProfile]);
      break;
    case NextionPage::BrewSoak:
      lcdFetchSoak(settings.profiles[targetProfile]);
      break;
    case NextionPage::BrewProfiling:
      lcdFetchBrewProfile(settings.profiles[targetProfile]);
      break;
    case NextionPage::BrewTransitionProfile:
      lcdFetchTransitionProfile(settings.profiles[targetProfile]);
      break;
    case NextionPage::SettingsBoiler:
      lcdFetchTemp(settings.profiles[targetProfile]);
      lcdFetchBoiler(settings);
      break;
    case NextionPage::SettingsSystem:
      lcdFetchSystem(settings);
      break;
    case NextionPage::ShotSettings:
      lcdFetchDoseSettings(settings.profiles[targetProfile]);
      break;
    default:
      break;
  }
}

uint8_t lcdGetSelectedProfile(void) {
  uint8_t pId;
  int attempts = 2;
  do {
    if (attempts-- <= 0) {
      lcdShowPopup((String("getProfile rekt: ") + pId).c_str());
      return 0;
    }
    pId = myNex.readNumber("pId");
  } while (pId < 1 || pId > 5);
  return pId - 1; /* 1-offset in nextion */
}

bool lcdGetPreinfusionFlowState(void) {
  return myNex.readNumber("piFlowState");
}

bool lcdGetProfileFlowState(void) {
  return myNex.readNumber("ppType");
}

bool lcdGetTransitionFlowState(void) {
  return myNex.readNumber("paType");
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

void lcdSetTemperatureDecimal(uint16_t val) {
  myNex.writeNum("dE.val", val);
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
  if(millis() > timer + 1450) {
    myNex.writeStr("popupMSG.t0.txt", msg);
    myNex.writeStr("page popupMSG");
    timer = millis();
  }
}

void lcdSetTankWaterLvl(uint16_t val) {
  myNex.writeNum("j0.val", val);
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

void lcdSetLedColour(SystemState& sys) {
  uint16_t colourCode = myNex.readNumber("ledID");
  uint8_t colourID;
  uint8_t colour;

  if (colourCode > 510) colourID = 3;
  else if (colourCode > 255) colourID = 2;
  else colourID = 1;

  switch (colourID) {
  case 0:
    ledCtrl.setColor(10,10,10);
    break;
  case 1:
    ledCtrl.setRed(colourCode);
    sys.ledColours[0] = colourCode;
    break;
  case 2:
    colour = colourCode - 255;
    sys.ledColours[1] = colour;
    ledCtrl.setGreen(colour);
    break;
  case 3:
    colour = colourCode - 510;
    sys.ledColours[2] = colour;
    ledCtrl.setBlue(colour);
    break;
  default:
    ledCtrl.setColor(0,0,0);
    break;
  }
}

void trigger1(void) { lcdSaveSettingsTrigger(); }
void trigger2(void) { lcdScalesTareTrigger(); }
void trigger3(void) { lcdHomeScreenScalesTrigger(); }
void trigger4(void) { lcdBrewGraphScalesTareTrigger(); }
void trigger6(void) { lcdRefreshElementsTrigger(); }
void trigger7(void) { lcdQuickProfileSwitch(); }
void trigger8(void) { lcdSaveProfileTrigger(); }
void trigger9(void) { lcdResetSettingsTrigger(); }
void trigger10(void) { lcdLoadDefaultProfileTrigger(); }
