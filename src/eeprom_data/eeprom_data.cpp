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

  eepromValues_t getEepromDefaults(uint8_t idx) {
    eepromValues_t defaultData[ACTIVE_PROFILES_TOTAL];
    // Default profile
    switch (idx) {
      case 0:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      case 1:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      case 2:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      case 3:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      case 4:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      case 5:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      case 6:
        defaultData[idx].idx = 0;
        // PI
        defaultData[idx].preinfusionState = true;
        defaultData[idx].preinfusionSec = 10;
        defaultData[idx].preinfusionBar = 2;
        defaultData[idx].preinfusionRamp = 0;
        defaultData[idx].preinfusionFlowState = false;
        defaultData[idx].preinfusionFlowVol = 3.5f;
        defaultData[idx].preinfusionFlowTime = 10;
        defaultData[idx].preinfusionFlowPressureTarget = 3.5f;
        defaultData[idx].preinfusionPressureFlowTarget = 3.5f;
        defaultData[idx].preinfusionFilled = 0;
        defaultData[idx].preinfusionPressureAbove = 0.f;
        defaultData[idx].preinfusionWeightAbove = 0.f;
        // SOAK
        defaultData[idx].soakState = 1;
        defaultData[idx].soakTimePressure = 10;
        defaultData[idx].soakTimeFlow = 5;
        defaultData[idx].soakKeepPressure = 0.f;
        defaultData[idx].soakKeepFlow = 0.f;
        defaultData[idx].soakBelowPressure = 0.f;
        defaultData[idx].soakAbovePressure = 0.f;
        defaultData[idx].soakAboveWeight = 0.f;
        // PI -> PF
        defaultData[idx].preinfusionRamp = 2;
        defaultData[idx].preinfusionRampSlope = 2;
        // Profiling
        defaultData[idx].profilingState = true;
        defaultData[idx].flowProfileState = false;
        defaultData[idx].pressureProfilingStart = 9;
        defaultData[idx].pressureProfilingFinish = 6;
        defaultData[idx].pressureProfilingHold = 5;
        defaultData[idx].pressureProfilingHoldLimit = 3.f;
        defaultData[idx].pressureProfilingSlope = 15;
        defaultData[idx].pressureProfilingFlowRestriction = 3.f;
        defaultData[idx].flowProfileStart = 3.5f;
        defaultData[idx].flowProfileEnd = 2.0f;
        defaultData[idx].flowProfileHold = 3.f;
        defaultData[idx].flowProfileHoldLimit = 9.f;
        defaultData[idx].flowProfileSlope = 15;
        defaultData[idx].flowProfilingPressureRestriction = 7;
        // General brew settings
        defaultData[idx].homeOnShotFinish = true;
        defaultData[idx].brewDeltaState = true;
        defaultData[idx].basketPrefill = false;
        // System settings
        defaultData[idx].setpoint = 93;
        defaultData[idx].steamSetPoint = 155;
        defaultData[idx].offsetTemp = 7;
        defaultData[idx].hpwr = 550;
        defaultData[idx].mainDivider = 5;
        defaultData[idx].brewDivider = 3;
        defaultData[idx].powerLineFrequency = 50;
        defaultData[idx].lcdSleep = 16;
        defaultData[idx].warmupState = false;
        defaultData[idx].scalesF1 = 3920;
        defaultData[idx].scalesF2 = 4210;
        defaultData[idx].pumpFlowAtZero = 0.2401f;
        // Dose settings
        defaultData[idx].stopOnWeightState = false;
        defaultData[idx].shotDose = 18.0f;
        defaultData[idx].shotStopOnCustomWeight = 0.f;
        defaultData[idx].shotPreset = 0;
        break;
      default:
        LOG_ERROR("EEPROM Defaults init error!");
        break;
    }

    return defaultData[idx];
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
  // selecting the last active profile
  uint8_t idx = eepromMetadata.lastActiveprofileIdx;
  // initialiaze defaults on memory
  eepromMetadata.values[idx] = getEepromDefaults(idx);

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
    eepromMetadata.values[idx] = getEepromDefaults(idx);
  }

  if (!readSuccess || version != EEPROM_DATA_VERSION) {
    eepromWrite(eepromMetadata.values[idx], eepromMetadata.lastActiveprofileIdx);
  }
  return eepromMetadata.lastActiveprofileIdx;
}

struct eepromValues_t eepromGetCurrentValues(uint8_t idx) {
  return eepromMetadata.values[idx];
}
