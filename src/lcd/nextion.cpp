/* 09:32 15/03/2023 - change triggering comment */
#include "lcd.h"
#include "pindef.h"
#include "log.h"
#include <Arduino.h>
#include "../eeprom_data/eeprom_data.h"
#include "nextion_profile_mapping.h"

EasyNex myNex(USART_LCD);
volatile NextionPage lcdCurrentPageId;
volatile NextionPage lcdLastCurrentPageId;
nextion_profile_t nextionProfile;

// decode/encode bit packing.
// format is 000000sd rrrrrrrr gggggggg bbbbbbbb, where s = state, d = disco, r/g/b = colors
void lcdDecodeLedSettings(uint32_t code, bool &state, bool &disco, uint8_t &r, uint8_t &g, uint8_t &b) {
  state = (code & 0x02000000);
  disco = (code & 0x01000000);
  r     = (code & 0x00FF0000) >> 16;
  g     = (code & 0x0000FF00) >> 8;
  b     = (code & 0x000000FF);
}

uint32_t lcdEncodeLedSettings(bool state, bool disco, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t code;
  code = state ? 0x01 : 0x00;
  code = (code << 1) | (disco ? 0x01 : 0x00);
  code = (code << 8) | (r & 0xFF);
  code = (code << 8) | (g & 0xFF);
  code = (code << 8) | (b & 0xFF);
  return code;
}

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
      }
      else {
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

void lcdUploadProfile(GaggiaSettings& settings) {
  mapProfileToNextionProfile(ACTIVE_PROFILE(settings), nextionProfile);

  // Highlight the active profile
  myNex.writeNum("pId", settings.profiles.activeProfileIndex + 1  /* 1-offset in nextion */);
  String buttonElemId = String("home.qPf") + (settings.profiles.activeProfileIndex + 1) + ".txt";
  myNex.writeStr(buttonElemId, nextionProfile.name);

  // Temp
  myNex.writeNum("sT.setPoint.val", nextionProfile.setpoint);
  // PI
  myNex.writeNum("piState", nextionProfile.preinfusionState);
  myNex.writeNum("piFlowState", nextionProfile.preinfusionFlowState);

  if (nextionProfile.preinfusionFlowState == 0) {
    myNex.writeNum("pi.piTime.val", nextionProfile.preinfusionSec);
    myNex.writeNum("pi.piFlow.val", nextionProfile.preinfusionPressureFlowTarget * 10.f);
    myNex.writeNum("pi.piBar.val", nextionProfile.preinfusionBar * 10.f);
  }
  else {
    myNex.writeNum("pi.piTime.val", nextionProfile.preinfusionFlowTime);
    myNex.writeNum("pi.piFlow.val", nextionProfile.preinfusionFlowVol * 10.f);
    myNex.writeNum("pi.piBar.val", nextionProfile.preinfusionFlowPressureTarget * 10.f);
  }
  myNex.writeNum("pi.piPumped.val", nextionProfile.preinfusionFilled);
  myNex.writeNum("piRPressure", nextionProfile.preinfusionPressureAbove);
  myNex.writeNum("pi.piAbove.val", nextionProfile.preinfusionWeightAbove * 10.f);

  // SOAK
  myNex.writeNum("skState", nextionProfile.soakState);

  if (nextionProfile.preinfusionFlowState == 0)
    myNex.writeNum("sk.skTime.val", nextionProfile.soakTimePressure);
  else
    myNex.writeNum("sk.skTime.val", nextionProfile.soakTimeFlow);

  myNex.writeNum("sk.skBar.val", nextionProfile.soakKeepPressure * 10.f);
  myNex.writeNum("sk.skFlow.val", nextionProfile.soakKeepFlow * 10.f);
  myNex.writeNum("sk.skBelow.val", nextionProfile.soakBelowPressure * 10.f);
  myNex.writeNum("sk.skAbv.val", nextionProfile.soakAbovePressure * 10.f);
  myNex.writeNum("sk.skWAbv.val", nextionProfile.soakAboveWeight * 10.f);
  // PI -> PF
  myNex.writeNum("sk.skRamp.val", nextionProfile.preinfusionRamp);
  myNex.writeNum("skCrv", nextionProfile.preinfusionRampSlope);
  // PROFILING
  // Adnvanced transition profile
  myNex.writeNum("paState", nextionProfile.tpState);
  myNex.writeNum("paType", nextionProfile.tpType);
  if (nextionProfile.tpType == 0) {
    myNex.writeNum("tp.tStart.val", nextionProfile.tpProfilingStart * 10.f);
    myNex.writeNum("tp.tEnd.val", nextionProfile.tpProfilingFinish * 10.f);
    myNex.writeNum("tp.tHold.val", nextionProfile.tpProfilingHold);
    myNex.writeNum("tp.hLim.val", nextionProfile.tpProfilingHoldLimit * 10.f);
    myNex.writeNum("tp.tSlope.val", nextionProfile.tpProfilingSlope);
    myNex.writeNum("paCrv", nextionProfile.tpProfilingSlopeShape);
    myNex.writeNum("tp.tLim.val", nextionProfile.tpProfilingFlowRestriction * 10.f);
  }
  else {
    myNex.writeNum("tp.tStart.val", nextionProfile.tfProfileStart * 10.f);
    myNex.writeNum("tp.tEnd.val", nextionProfile.tfProfileEnd * 10.f);
    myNex.writeNum("tp.tHold.val", nextionProfile.tfProfileHold);
    myNex.writeNum("tp.hLim.val", nextionProfile.tfProfileHoldLimit * 10.f);
    myNex.writeNum("tp.tSlope.val", nextionProfile.tfProfileSlope);
    myNex.writeNum("paCrv", nextionProfile.tfProfileSlopeShape);
    myNex.writeNum("tp.tLim.val", nextionProfile.tfProfilingPressureRestriction * 10.f);
  }
  // Main profile
  myNex.writeNum("ppState", nextionProfile.profilingState);
  myNex.writeNum("ppType", nextionProfile.mfProfileState);
  if (nextionProfile.mfProfileState == 0) {
    myNex.writeNum("pf.pStart.val", nextionProfile.mpProfilingStart * 10.f);
    myNex.writeNum("pf.pEnd.val", nextionProfile.mpProfilingFinish * 10.f);
    myNex.writeNum("pf.pSlope.val", nextionProfile.mpProfilingSlope);
    myNex.writeNum("pfCrv", nextionProfile.mpProfilingSlopeShape);
    myNex.writeNum("pf.pLim.val", nextionProfile.mpProfilingFlowRestriction * 10.f);
  }
  else {
    myNex.writeNum("pf.pStart.val", nextionProfile.mfProfileStart * 10.f);
    myNex.writeNum("pf.pEnd.val", nextionProfile.mfProfileEnd * 10.f);
    myNex.writeNum("pf.pSlope.val", nextionProfile.mfProfileSlope);
    myNex.writeNum("pfCrv", nextionProfile.mfProfileSlopeShape);
    myNex.writeNum("pf.pLim.val", nextionProfile.mfProfilingPressureRestriction * 10.f);
  }
  // Dose settings
  myNex.writeNum("shotState", nextionProfile.stopOnWeightState);
  myNex.writeNum("dS.numDose.val", nextionProfile.shotDose * 10.f);
  myNex.writeNum("shotPreset", nextionProfile.shotPreset);
  myNex.writeNum("dS.numDoseForced.val", nextionProfile.shotStopOnCustomWeight * 10.f);
}

// This is never called again after boot
void lcdUploadCfg(GaggiaSettings& settings) {
  // Profile names for all buttons
  myNex.writeStr("home.qPf1.txt", settings.profiles.savedProfiles[0].name.c_str());
  myNex.writeStr("home.qPf2.txt", settings.profiles.savedProfiles[1].name.c_str());
  myNex.writeStr("home.qPf3.txt", settings.profiles.savedProfiles[2].name.c_str());
  myNex.writeStr("home.qPf4.txt", settings.profiles.savedProfiles[3].name.c_str());
  myNex.writeStr("home.qPf5.txt", settings.profiles.savedProfiles[4].name.c_str());

  // More brew settings
  myNex.writeNum("bckHome", settings.brew.homeOnShotFinish);
  myNex.writeNum("deltaState", settings.brew.brewDeltaState);
  myNex.writeNum("basketPrefill", settings.brew.basketPrefill);

  // System settings
  myNex.writeNum("sT.steamSetPoint.val", settings.boiler.steamSetPoint);
  myNex.writeNum("sT.offSet.val", settings.boiler.offsetTemp);
  myNex.writeNum("sT.hpwr.val", settings.boiler.hpwr);
  myNex.writeNum("sT.mDiv.val", settings.boiler.mainDivider);
  myNex.writeNum("sT.bDiv.val", settings.boiler.brewDivider);

  myNex.writeNum("sP.n1.val", settings.system.lcdSleep);
  myNex.writeNum("sP.lc1.val", settings.system.scalesF1);
  myNex.writeNum("sP.lc2.val", settings.system.scalesF2);
  myNex.writeNum("sP.pump_zero.val", settings.system.pumpFlowAtZero * 10000.f);
  myNex.writeNum("warmupState", settings.system.warmupState);

  // Led
  myNex.writeNum("ledNum",
    lcdEncodeLedSettings(
      settings.led.state,
      settings.led.disco,
      settings.led.color.R,
      settings.led.color.G,
      settings.led.color.B
    )
  );
  lcdUploadProfile(settings);
}

void uploadPageCfg(GaggiaSettings& settings, SystemState& sys) {

  // Updating only page specific elements as necessary to speed up things and avoid needless writes.
  mapProfileToNextionProfile(ACTIVE_PROFILE(settings), nextionProfile);

  switch (lcdCurrentPageId) {
  case NextionPage::BrewPreinfusion:
    // PI
    mapPreinfusionPhaseToNextion(ACTIVE_PROFILE(settings), nextionProfile);
    myNex.writeNum("piState", nextionProfile.preinfusionState);
    myNex.writeNum("piFlowState", nextionProfile.preinfusionFlowState);

    if (nextionProfile.preinfusionFlowState == 0) {
      myNex.writeNum("pi.piTime.val", nextionProfile.preinfusionSec);
      myNex.writeNum("pi.piFlow.val", nextionProfile.preinfusionPressureFlowTarget * 10.f);
      myNex.writeNum("pi.piBar.val", nextionProfile.preinfusionBar * 10.f);
    }
    else {
      myNex.writeNum("pi.piTime.val", nextionProfile.preinfusionFlowTime);
      myNex.writeNum("pi.piFlow.val", nextionProfile.preinfusionFlowVol * 10.f);
      myNex.writeNum("pi.piBar.val", nextionProfile.preinfusionFlowPressureTarget * 10.f);
    }
    myNex.writeNum("pi.piPumped.val", nextionProfile.preinfusionFilled);
    myNex.writeNum("piRPressure", nextionProfile.preinfusionPressureAbove);
    myNex.writeNum("pi.piAbove.val", nextionProfile.preinfusionWeightAbove * 10.f);
    break;
  case NextionPage::BrewSoak:
    mapSoakPhaseToNextion(ACTIVE_PROFILE(settings), nextionProfile);
    mapRampPhaseToNextion(ACTIVE_PROFILE(settings), nextionProfile);
    myNex.writeNum("skState", nextionProfile.soakState);

    if (nextionProfile.preinfusionFlowState == 0) {
      myNex.writeNum("sk.skTime.val", nextionProfile.soakTimePressure);
    }
    else {
      myNex.writeNum("sk.skTime.val", nextionProfile.soakTimeFlow);
      myNex.writeNum("sk.skBar.val", nextionProfile.soakKeepPressure * 10.f);
      myNex.writeNum("sk.skFlow.val", nextionProfile.soakKeepFlow * 10.f);
      myNex.writeNum("sk.skBelow.val", nextionProfile.soakBelowPressure * 10.f);
      myNex.writeNum("sk.skAbv.val", nextionProfile.soakAbovePressure * 10.f);
      myNex.writeNum("sk.skWAbv.val", nextionProfile.soakAboveWeight * 10.f);
      // PI -> PF
      myNex.writeNum("sk.skRamp.val", nextionProfile.preinfusionRamp);
      myNex.writeNum("skCrv", nextionProfile.preinfusionRampSlope);
    }
    break;
  case NextionPage::BrewProfiling:
    // PROFILING
    myNex.writeNum("ppState", nextionProfile.profilingState);
    myNex.writeNum("ppType", nextionProfile.mfProfileState);

    if (nextionProfile.mfProfileState == 0) {
      myNex.writeNum("pf.pStart.val", nextionProfile.mpProfilingStart * 10.f);
      myNex.writeNum("pf.pEnd.val", nextionProfile.mpProfilingFinish * 10.f);
      myNex.writeNum("pf.pSlope.val", nextionProfile.mpProfilingSlope);
      myNex.writeNum("pfCrv", nextionProfile.mpProfilingSlopeShape);
      myNex.writeNum("pf.pLim.val", nextionProfile.mpProfilingFlowRestriction * 10.f);
    }
    else {
      myNex.writeNum("pf.pStart.val", nextionProfile.mfProfileStart * 10.f);
      myNex.writeNum("pf.pEnd.val", nextionProfile.mfProfileEnd * 10.f);
      myNex.writeNum("pf.pSlope.val", nextionProfile.mfProfileSlope);
      myNex.writeNum("pfCrv", nextionProfile.mfProfileSlopeShape);
      myNex.writeNum("pf.pLim.val", nextionProfile.mfProfilingPressureRestriction * 10.f);
    }
    break;
  case NextionPage::BrewTransitionProfile:
    myNex.writeNum("paState", nextionProfile.tpState);
    myNex.writeNum("paType", nextionProfile.tpType);
    // Adnvanced transition profile
    if (nextionProfile.tpType == 0) {
      myNex.writeNum("tp.tStart.val", nextionProfile.tpProfilingStart * 10.f);
      myNex.writeNum("tp.tEnd.val", nextionProfile.tpProfilingFinish * 10.f);
      myNex.writeNum("tp.tHold.val", nextionProfile.tpProfilingHold);
      myNex.writeNum("tp.hLim.val", nextionProfile.tpProfilingHoldLimit * 10.f);
      myNex.writeNum("tp.tSlope.val", nextionProfile.tpProfilingSlope);
      myNex.writeNum("paCrv", nextionProfile.tpProfilingSlopeShape);
      myNex.writeNum("tp.tLim.val", nextionProfile.tpProfilingFlowRestriction * 10.f);
    }
    else {
      myNex.writeNum("tp.tStart.val", nextionProfile.tfProfileStart * 10.f);
      myNex.writeNum("tp.tEnd.val", nextionProfile.tfProfileEnd * 10.f);
      myNex.writeNum("tp.tHold.val", nextionProfile.tfProfileHold);
      myNex.writeNum("tp.hLim.val", nextionProfile.tfProfileHoldLimit * 10.f);
      myNex.writeNum("tp.tSlope.val", nextionProfile.tfProfileSlope);
      myNex.writeNum("paCrv", nextionProfile.tfProfileSlopeShape);
      myNex.writeNum("tp.tLim.val", nextionProfile.tfProfilingPressureRestriction * 10.f);
    }
    break;
  default:
    lcdUploadCfg(settings);
    break;
  }
}

void lcdFetchProfileName(nextion_profile_t& profile, uint8_t index /* 0-offset */) {
  String buttonElemId = String("home.qPf") + (index + 1) + ".txt";
  snprintf(profile.name, sizeof(profile.name), "%s", myNex.readStr(buttonElemId).c_str());
}

void lcdFetchPreinfusion(nextion_profile_t& profile) {
  profile.preinfusionState = myNex.readNumber("piState");
  profile.preinfusionFlowState = lcdGetPreinfusionFlowState();

  if (profile.preinfusionFlowState == 0) {
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

void lcdFetchSoak(nextion_profile_t& profile) {
  // SOAK
  profile.soakState = myNex.readNumber("skState");

  if (profile.preinfusionFlowState == 0)
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

void lcdFetchBrewProfile(nextion_profile_t& profile) {
  // PROFILING
  profile.profilingState = myNex.readNumber("ppState");
  profile.mfProfileState = lcdGetProfileFlowState();

  if (profile.mfProfileState == 0) {
    profile.mpProfilingStart = myNex.readNumber("pf.pStart.val") / 10.f;
    profile.mpProfilingFinish = myNex.readNumber("pf.pEnd.val") / 10.f;
    profile.mpProfilingSlope = myNex.readNumber("pf.pSlope.val");
    profile.mpProfilingSlopeShape = myNex.readNumber("pfCrv");
    profile.mpProfilingFlowRestriction = myNex.readNumber("pf.pLim.val") / 10.f;
  }
  else {
    profile.mfProfileStart = myNex.readNumber("pf.pStart.val") / 10.f;
    profile.mfProfileEnd = myNex.readNumber("pf.pEnd.val") / 10.f;
    profile.mfProfileSlope = myNex.readNumber("pf.pSlope.val");
    profile.mfProfileSlopeShape = myNex.readNumber("pfCrv");
    profile.mfProfilingPressureRestriction = myNex.readNumber("pf.pLim.val") / 10.f;
  }
}

void lcdFetchTransitionProfile(nextion_profile_t& profile) {
  profile.tpState = myNex.readNumber("paState");
  profile.tpType = lcdGetTransitionFlowState();

  if (profile.tpType == 0) {
    profile.tpProfilingStart = myNex.readNumber("tp.tStart.val") / 10.f;
    profile.tpProfilingFinish = myNex.readNumber("tp.tEnd.val") / 10.f;
    profile.tpProfilingHold = myNex.readNumber("tp.tHold.val");
    profile.tpProfilingHoldLimit = myNex.readNumber("tp.hLim.val") / 10.f;
    profile.tpProfilingSlope = myNex.readNumber("tp.tSlope.val");
    profile.tpProfilingSlopeShape = myNex.readNumber("paCrv");
    profile.tpProfilingFlowRestriction = myNex.readNumber("tp.tLim.val") / 10.f;
  }
  else {
    profile.tfProfileStart = myNex.readNumber("tp.tStart.val") / 10.f;
    profile.tfProfileEnd = myNex.readNumber("tp.tEnd.val") / 10.f;
    profile.tfProfileHold = myNex.readNumber("tp.tHold.val");
    profile.tfProfileHoldLimit = myNex.readNumber("tp.tLim.val") / 10.f;
    profile.tfProfileSlope = myNex.readNumber("tp.tSlope.val");
    profile.tfProfileSlopeShape = myNex.readNumber("paCrv");
    profile.tfProfilingPressureRestriction = myNex.readNumber("tp.tLim.val") / 10.f;
  }
}

void lcdFetchDoseSettings(nextion_profile_t& profile) {
  // DOse settings
  profile.stopOnWeightState = myNex.readNumber("shotState");
  profile.shotDose = myNex.readNumber("dS.numDose.val") / 10.f;
  profile.shotStopOnCustomWeight = myNex.readNumber("dS.numDoseForced.val") / 10.f;
  profile.shotPreset = myNex.readNumber("shotPreset");
}

void lcdFetchTemp(nextion_profile_t& profile) {
  profile.setpoint = myNex.readNumber("sT.setPoint.val");
}

/**
* Overwrites the entire profile in the index corresponding to
* the currently selected profile on the screen.
*/
void lcdFetchCurrentProfile(GaggiaSettings& settings) {
  // Target save to the currently selected profile on screen (can be different from runningCfg on long press)
  settings.profiles.activeProfileIndex = lcdGetSelectedProfile();

  lcdFetchProfileName(nextionProfile, settings.profiles.activeProfileIndex);
  lcdFetchPreinfusion(nextionProfile);
  lcdFetchSoak(nextionProfile);
  lcdFetchBrewProfile(nextionProfile);
  lcdFetchTransitionProfile(nextionProfile);
  lcdFetchDoseSettings(nextionProfile);
  lcdFetchTemp(nextionProfile);

  mapNextionProfileToProfile(nextionProfile, ACTIVE_PROFILE(settings));
}

void lcdFetchBrewSettings(GaggiaSettings& settings) {
  // More brew settings
  settings.brew.homeOnShotFinish = myNex.readNumber("bckHome");
  settings.brew.basketPrefill = myNex.readNumber("basketPrefill");
  settings.brew.brewDeltaState = myNex.readNumber("deltaState");
}

void lcdFetchBoiler(GaggiaSettings& settings) {
  settings.boiler.steamSetPoint = myNex.readNumber("sT.steamSetPoint.val");
  settings.boiler.offsetTemp = myNex.readNumber("sT.offSet.val");
  settings.boiler.hpwr = myNex.readNumber("sT.hpwr.val");
  settings.boiler.mainDivider = myNex.readNumber("sT.mDiv.val");
  settings.boiler.brewDivider = myNex.readNumber("sT.bDiv.val");
}

void lcdFetchSystem(GaggiaSettings& settings) {
  // System settings
  settings.system.lcdSleep = myNex.readNumber("sP.n1.val"); // nextion sleep var
  settings.system.warmupState = myNex.readNumber("warmupState");
  settings.system.scalesF1 = myNex.readNumber("sP.lc1.val");
  settings.system.scalesF2 = myNex.readNumber("sP.lc2.val");
  settings.system.pumpFlowAtZero = myNex.readNumber("sP.pump_zero.val") / 10000.f;
}

void lcdFetchLed(eepromValues_t &settings) {
  // Led Settings
  uint32_t ledNum = myNex.readNumber("ledNum");
  lcdDecodeLedSettings(ledNum, settings.led.state, settings.led.disco, settings.led.color.R, settings.led.color.G, settings.led.color.B);
}

void lcdFetchPage(GaggiaSettings& settings, NextionPage page, int targetProfile) {
  switch (page) {
  case NextionPage::BrewMore:
    lcdFetchBrewSettings(settings);
    break;
  case NextionPage::BrewPreinfusion:
    lcdFetchPreinfusion(nextionProfile);
    mapPreinfusionPhaseFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    break;
  case NextionPage::BrewSoak:
    lcdFetchSoak(nextionProfile);
    mapSoakPhaseFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    mapRampPhaseFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    break;
  case NextionPage::BrewProfiling:
    lcdFetchBrewProfile(nextionProfile);
    mapMainSlopeFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    break;
  case NextionPage::BrewTransitionProfile:
    lcdFetchTransitionProfile(nextionProfile);
    mapTransitionPhaseFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    break;
  case NextionPage::SettingsBoiler:
    lcdFetchTemp(nextionProfile);
    mapGlobalVarsFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    lcdFetchBoiler(settings);
    break;
  case NextionPage::SettingsSystem:
    lcdFetchSystem(settings);
    break;
  case NextionPage::ShotSettings:
    lcdFetchDoseSettings(nextionProfile);
    mapGlobalVarsFromNextion(nextionProfile, settings.profiles.savedProfiles[targetProfile]);
    break;
  case NextionPage::Led:
    lcdFetchLed(settings);
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
  myNex.writeNum("debug1", val1);
  myNex.writeNum("debug2", val2);
}

void lcdShowPopup(String msg) {
  static unsigned int timer;
  if (millis() > timer + 1150) {
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

uint16_t lcdGetSliderColour(void) {
  return myNex.readNumber("ledNum"); // reading the slider colour code
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
