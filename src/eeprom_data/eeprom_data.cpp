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
#include "legacy/eeprom_data_v9.h"

namespace {

  struct eepromMetadata_t eepromMetadata;

  eepromValues_t getEepromDefaults(void) {
    eepromValues_t defaultData;
    // Default profile
    defaultData.idx = 0;
    // PI
    defaultData.preinfusionState = true;
    defaultData.preinfusionSec = 10;
    defaultData.preinfusionBar = 2;
    defaultData.preinfusionRamp = 0;
    defaultData.preinfusionFlowState = false;
    defaultData.preinfusionFlowVol = 3.5f;
    defaultData.preinfusionFlowTime = 10;
    defaultData.preinfusionFlowPressureTarget = 3.5f;
    defaultData.preinfusionPressureFlowTarget = 3.5f;
    defaultData.preinfusionFilled = 0;
    defaultData.preinfusionPressureAbove = 0.f;
    defaultData.preinfusionWeightAbove = 0.f;
    // SOAK
    defaultData.soakState = 1;
    defaultData.soakTimePressure = 10;
    defaultData.soakTimeFlow = 5;
    defaultData.soakKeepPressure = 0.f;
    defaultData.soakKeepFlow = 0.f;
    defaultData.soakBelowPressure = 0.f;
    defaultData.soakAbovePressure = 0.f;
    defaultData.soakAboveWeight = 0.f;
    // PI -> PF
    defaultData.preinfusionRamp = 2;
    defaultData.preinfusionRampSlope = 2;
    // Profiling
    defaultData.profilingState = true;
    defaultData.flowProfileState = false;
    defaultData.pressureProfilingStart = 9;
    defaultData.pressureProfilingFinish = 6;
    defaultData.pressureProfilingHold = 5;
    defaultData.pressureProfilingHoldLimit = 3.f;
    defaultData.pressureProfilingSlope = 15;
    defaultData.pressureProfilingFlowRestriction = 3.f;
    defaultData.flowProfileStart = 3.5f;
    defaultData.flowProfileEnd = 2.0f;
    defaultData.flowProfileHold = 3.f;
    defaultData.flowProfileHoldLimit = 9.f;
    defaultData.flowProfileSlope = 15;
    defaultData.flowProfilingPressureRestriction = 7;
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

bool eepromWrite(eepromValues_t eepromValuesNew, uint8_t idx) {
  const char *errMsg = "Data out of range";

  if (eepromValuesNew.preinfusionFlowVol < 0.f) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.flowProfileStart < 0.f) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.flowProfileEnd < 0.f) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.pressureProfilingStart < 0.f) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.pressureProfilingFinish < 0.f) {
    LOG_ERROR(errMsg);
    return false;
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
  eepromMetadata.values[idx] = eepromValuesNew;
  eepromMetadata.lastActiveprofileIdx = idx;
  eepromMetadata.versionTimestampXOR = eepromMetadata.timestamp ^ eepromMetadata.version;
  EEPROM.put(0, eepromMetadata);

  return true;
}

uint8_t eepromInit(void) {
  uint8_t idx;
  // selecting the last active profile
  idx = eepromMetadata.lastActiveprofileIdx;
  // initialiaze defaults on memory
  eepromMetadata.values[idx] = getEepromDefaults();

  // read version
  uint16_t version;
  EEPROM.get(0, version);

  // load appropriate version (including current)
  bool readSuccess = false;

  if (version < EEPROM_DATA_VERSION && legacyEepromDataLoaders[version] != nullptr) {
    readSuccess = (*legacyEepromDataLoaders[version])(eepromMetadata.values[idx]);
  } else {
    readSuccess = loadCurrentEepromData(eepromMetadata.values[idx]);
  }

  if (!readSuccess) {
    LOG_ERROR("SECU_CHECK FAILED! Applying defaults! eepromMetadata.version=%d", version);
    eepromMetadata.values[idx] = getEepromDefaults();
  }

  if (!readSuccess || version != EEPROM_DATA_VERSION) {
    eepromWrite(eepromMetadata.values[idx], eepromMetadata.lastActiveprofileIdx);
  }
  return eepromMetadata.lastActiveprofileIdx;
}

struct eepromValues_t eepromGetCurrentValues(uint8_t idx) {
  return eepromMetadata.values[idx];
}
