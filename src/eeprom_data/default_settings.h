#ifndef GAGGIA_DEFAULT_SETTINGS_H
#define GAGGIA_DEFAULT_SETTINGS_H

#include "gaggia_settings.h"
#include "new_default_profiles.h"

GaggiaSettings getDefaultGaggiaSettings(void) {
  GaggiaSettings defaultData;

  // Profiles
  defaultData.profiles.activeProfileIndex = 0;
  defaultData.profiles.savedProfiles.clear();
  for (auto profile : defaultProfiles) {
    defaultData.profiles.savedProfiles.push_back(Profile(profile));
  }

  // Boiler
  defaultData.boiler.steamSetPoint = 155;
  defaultData.boiler.offsetTemp = 7;
  defaultData.boiler.hpwr = 550;
  defaultData.boiler.mainDivider = 5;
  defaultData.boiler.brewDivider = 3;

  // Screen
  defaultData.brew.homeOnShotFinish = false;
  defaultData.brew.brewDeltaState = true;
  defaultData.brew.basketPrefill = false;

  // System settings
  defaultData.system.powerLineFrequency = 50;
  defaultData.system.scalesF1 = 3920;
  defaultData.system.scalesF2 = 4210;
  defaultData.system.pumpFlowAtZero = 0.2401f;
  defaultData.system.lcdSleep = 16;
  defaultData.system.warmupState = false;

  // LED settings
  defaultData.led.state = true;
  defaultData.led.color.R = 9;
  defaultData.led.color.G = 0;
  defaultData.led.color.B = 9;

  return defaultData;
}

#endif
