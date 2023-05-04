/* 09:32 15/03/2023 - change triggering comment */
#define STM32F4 // This define has to be here otherwise the include of FlashStorage_STM32.h bellow fails.
#include <FlashStorage_STM32.h>
#include "eeprom_data.h"
#include "eeprom_metadata.h"
#include "legacy/eeprom_data_v4.h"
#include "legacy/eeprom_data_v5.h"
#include "legacy/eeprom_data_v6.h"
#include "legacy/eeprom_data_v7.h"
#include "legacy/eeprom_data_v8.h"

namespace {

  struct eepromMetadata_t eepromMetadata;

  eepromValues_t getEepromDefaults(void) {
    eepromValues_t defaultData;
    // Profiles
    defaultData.activeProfile = 0;
    snprintf(defaultData.profiles[0].name, 50, "%s", "Default");
    snprintf(defaultData.profiles[1].name, 50, "%s", "TBD");
    snprintf(defaultData.profiles[2].name, 50, "%s", "TBD");
    snprintf(defaultData.profiles[3].name, 50, "%s", "TBD");
    snprintf(defaultData.profiles[4].name, 50, "%s", "TBD");
    // TODO: create actual default profiles 1-4
    for (int i=0; i<MAX_PROFILES; i++) {
      // PI
      defaultData.profiles[i].preinfusionState = true;
      defaultData.profiles[i].preinfusionSec = 10;
      defaultData.profiles[i].preinfusionBar = 2;
      defaultData.profiles[i].preinfusionRamp = 0;
      defaultData.profiles[i].preinfusionFlowState = false;
      defaultData.profiles[i].preinfusionFlowVol = 3.5f;
      defaultData.profiles[i].preinfusionFlowTime = 10;
      defaultData.profiles[i].preinfusionFlowPressureTarget = 3.5f;
      defaultData.profiles[i].preinfusionPressureFlowTarget = 3.5f;
      defaultData.profiles[i].preinfusionFilled = 0;
      defaultData.profiles[i].preinfusionPressureAbove = 0.f;
      defaultData.profiles[i].preinfusionWeightAbove = 0.f;
      // SOAK
      defaultData.profiles[i].soakState = 1;
      defaultData.profiles[i].soakTimePressure = 10;
      defaultData.profiles[i].soakTimeFlow = 5;
      defaultData.profiles[i].soakKeepPressure = 0.f;
      defaultData.profiles[i].soakKeepFlow = 0.f;
      defaultData.profiles[i].soakBelowPressure = 0.f;
      defaultData.profiles[i].soakAbovePressure = 0.f;
      defaultData.profiles[i].soakAboveWeight = 0.f;
      // PI -> PF
      defaultData.profiles[i].preinfusionRamp = 2;
      defaultData.profiles[i].preinfusionRampSlope = 2;
      // Profiling
      defaultData.profiles[i].profilingState = true;
      defaultData.profiles[i].flowProfileState = false;
      defaultData.profiles[i].pressureProfilingStart = 9;
      defaultData.profiles[i].pressureProfilingFinish = 6;
      defaultData.profiles[i].pressureProfilingHold = 5;
      defaultData.profiles[i].pressureProfilingHoldLimit = 3.f;
      defaultData.profiles[i].pressureProfilingSlope = 15;
      defaultData.profiles[i].pressureProfilingFlowRestriction = 3.f;
      defaultData.profiles[i].flowProfileStart = 3.5f;
      defaultData.profiles[i].flowProfileEnd = 2.0f;
      defaultData.profiles[i].flowProfileHold = 3.f;
      defaultData.profiles[i].flowProfileHoldLimit = 9.f;
      defaultData.profiles[i].flowProfileSlope = 15;
      defaultData.profiles[i].flowProfilingPressureRestriction = 7;
    }
    // General brew settings
    defaultData.homeOnShotFinish = true;
    defaultData.brewDeltaState = true;
    defaultData.basketPrefill = false;
    // System settings
    defaultData.setpoint = 93;
    defaultData.steamSetPoint = 155;
    defaultData.offsetTemp = 7;
    defaultData.hpwr = 550;
    defaultData.mainDivider = 5;
    defaultData.brewDivider = 3;
    defaultData.powerLineFrequency = 50;
    defaultData.lcdSleep = 16;
    defaultData.warmupState = false;
    defaultData.scalesF1 = 3920;
    defaultData.scalesF2 = 4210;
    defaultData.pumpFlowAtZero = 0.2401f;
    // Dose settings
    defaultData.stopOnWeightState = false;
    defaultData.shotDose = 18.0f;
    defaultData.shotStopOnCustomWeight = 0.f;
    defaultData.shotPreset = 0;

    return defaultData;
  }

  // kind of annoying, but allows reusing macro without messing up type safety
  template <typename T>
  bool copy_t(T& target, T& source) {
    target = source;
    return true;
  }

  bool loadCurrentEepromData EEPROM_METADATA_LOADER(EEPROM_DATA_VERSION, eepromMetadata_t, copy_t);

}

bool eepromWrite(eepromValues_t eepromValuesNew) {
  const char *errMsg = "Data out of range";

  for (int i=0; i<MAX_PROFILES; i++) {
    if (eepromValuesNew.profiles[i].preinfusionFlowVol < 0.f) {
      LOG_ERROR(errMsg);
      return false;
    }

    if (eepromValuesNew.profiles[i].flowProfileStart < 0.f) {
      LOG_ERROR(errMsg);
      return false;
    }

    if (eepromValuesNew.profiles[i].flowProfileEnd < 0.f) {
      LOG_ERROR(errMsg);
      return false;
    }

    if (eepromValuesNew.profiles[i].pressureProfilingStart < 0.f) {
      LOG_ERROR(errMsg);
      return false;
    }

    if (eepromValuesNew.profiles[i].pressureProfilingFinish < 0.f) {
      LOG_ERROR(errMsg);
      return false;
    }
  }

  if (eepromValuesNew.setpoint < 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.steamSetPoint < 1 || eepromValuesNew.steamSetPoint > 165) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.mainDivider < 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.brewDivider < 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.pumpFlowAtZero < 0.210f || eepromValuesNew.pumpFlowAtZero > 0.310f) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.scalesF1 < -20000 || eepromValuesNew.scalesF1 > 20000) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.scalesF2 < -20000 || eepromValuesNew.scalesF2 > 20000) {
    LOG_ERROR(errMsg);
    return false;
  }

  eepromMetadata.timestamp = millis();
  eepromMetadata.version = EEPROM_DATA_VERSION;
  eepromMetadata.values = eepromValuesNew;
  eepromMetadata.versionTimestampXOR = eepromMetadata.timestamp ^ eepromMetadata.version;
  EEPROM.put(0, eepromMetadata);

  return true;
}

void eepromInit(void) {
  // initialiaze defaults on memory
  eepromMetadata.values = getEepromDefaults();

  // read version
  uint16_t version;
  EEPROM.get(0, version);

  // load appropriate version (including current)
  bool readSuccess = false;

  if (version < EEPROM_DATA_VERSION && legacyEepromDataLoaders[version] != nullptr) {
    readSuccess = (*legacyEepromDataLoaders[version])(eepromMetadata.values);
  } else {
    readSuccess = loadCurrentEepromData(eepromMetadata.values);
  }

  if (!readSuccess) {
    LOG_ERROR("SECU_CHECK FAILED! Applying defaults! eepromMetadata.version=%d", version);
    eepromMetadata.values = getEepromDefaults();
  }

  if (!readSuccess || version != EEPROM_DATA_VERSION) {
    eepromWrite(eepromMetadata.values);
  }
}

struct eepromValues_t eepromGetCurrentValues(void) {
  return eepromMetadata.values;
}
