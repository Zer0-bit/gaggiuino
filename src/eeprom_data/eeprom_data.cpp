/* 09:32 15/03/2023 - change triggering comment */
#define STM32F4 // This define has to be here otherwise the include of FlashStorage_STM32.h bellow fails.
#include <FlashStorage_STM32.h>
#include "eeprom_data.h"
#include "eeprom_metadata.h"
#include "default_profiles.h"
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
    for (int i=0; i<MAX_PROFILES; i++) {
      snprintf(defaultData.profiles[i].name, 50, "%s", defaultsProfile[i].name);
      // PI
      defaultData.profiles[i].preinfusionState = defaultsProfile[i].preinfusionState;
      defaultData.profiles[i].preinfusionSec = defaultsProfile[i].preinfusionSec;
      defaultData.profiles[i].preinfusionBar = defaultsProfile[i].preinfusionBar;
      defaultData.profiles[i].preinfusionRamp = defaultsProfile[i].preinfusionRamp;
      defaultData.profiles[i].preinfusionFlowState = defaultsProfile[i].preinfusionFlowState;
      defaultData.profiles[i].preinfusionFlowVol = defaultsProfile[i].preinfusionFlowVol;
      defaultData.profiles[i].preinfusionFlowTime = defaultsProfile[i].preinfusionFlowTime;
      defaultData.profiles[i].preinfusionFlowPressureTarget = defaultsProfile[i].preinfusionFlowPressureTarget;
      defaultData.profiles[i].preinfusionPressureFlowTarget = defaultsProfile[i].preinfusionPressureFlowTarget;
      defaultData.profiles[i].preinfusionFilled = defaultsProfile[i].preinfusionFilled;
      defaultData.profiles[i].preinfusionPressureAbove = defaultsProfile[i].preinfusionPressureAbove;
      defaultData.profiles[i].preinfusionWeightAbove = defaultsProfile[i].preinfusionWeightAbove;
      // SOAK
      defaultData.profiles[i].soakState = defaultsProfile[i].soakState;
      defaultData.profiles[i].soakTimePressure = defaultsProfile[i].soakTimePressure;
      defaultData.profiles[i].soakTimeFlow = defaultsProfile[i].soakTimeFlow;
      defaultData.profiles[i].soakKeepPressure = defaultsProfile[i].soakKeepPressure;
      defaultData.profiles[i].soakKeepFlow = defaultsProfile[i].soakKeepFlow;
      defaultData.profiles[i].soakBelowPressure = defaultsProfile[i].soakBelowPressure;
      defaultData.profiles[i].soakAbovePressure = defaultsProfile[i].soakAbovePressure;
      defaultData.profiles[i].soakAboveWeight = defaultsProfile[i].soakAboveWeight;
      // PI -> PF
      defaultData.profiles[i].preinfusionRamp = defaultsProfile[i].preinfusionRamp;
      defaultData.profiles[i].preinfusionRampSlope = defaultsProfile[i].preinfusionRampSlope;
      // Profiling
      defaultData.profiles[i].profilingState = defaultsProfile[i].profilingState;
      defaultData.profiles[i].flowProfileState = defaultsProfile[i].flowProfileState;
      defaultData.profiles[i].pressureProfilingStart = defaultsProfile[i].pressureProfilingStart;
      defaultData.profiles[i].pressureProfilingFinish = defaultsProfile[i].pressureProfilingFinish;
      defaultData.profiles[i].pressureProfilingHold = defaultsProfile[i].pressureProfilingHold;
      defaultData.profiles[i].pressureProfilingHoldLimit = defaultsProfile[i].pressureProfilingHoldLimit;
      defaultData.profiles[i].pressureProfilingSlope = defaultsProfile[i].pressureProfilingSlope;
      defaultData.profiles[i].pressureProfilingSlopeShape = defaultsProfile[i].pressureProfilingSlopeShape;
      defaultData.profiles[i].pressureProfilingFlowRestriction = defaultsProfile[i].pressureProfilingFlowRestriction;
      defaultData.profiles[i].flowProfileStart = defaultsProfile[i].flowProfileStart;
      defaultData.profiles[i].flowProfileEnd = defaultsProfile[i].flowProfileEnd;
      defaultData.profiles[i].flowProfileHold = defaultsProfile[i].flowProfileHold;
      defaultData.profiles[i].flowProfileHoldLimit = defaultsProfile[i].flowProfileHoldLimit;
      defaultData.profiles[i].flowProfileSlope = defaultsProfile[i].flowProfileSlope;
      defaultData.profiles[i].flowProfileSlopeShape = defaultsProfile[i].flowProfileSlopeShape;
      defaultData.profiles[i].flowProfilingPressureRestriction = defaultsProfile[i].flowProfilingPressureRestriction;
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
