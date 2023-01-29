#define STM32F4 // This define has to be here otherwise the include of FlashStorage_STM32.h bellow fails.
#include <FlashStorage_STM32.h>
#include "eeprom_data.h"
#include "eeprom_metadata.h"
#include "legacy/eeprom_data_v4.h"
#include "legacy/eeprom_data_v5.h"

namespace {

  static struct eepromMetadata_t eepromMetadata;

  eepromValues_t getEepromDefaults(void) {
    eepromValues_t defaultData;

    defaultData.setpoint = 100;
    defaultData.steamSetPoint = 162;
    defaultData.offsetTemp = 7;
    defaultData.hpwr = 550;
    defaultData.mainDivider = 5;
    defaultData.brewDivider = 2;
    defaultData.pressureProfilingStart = 9;
    defaultData.pressureProfilingFinish = 6;
    defaultData.pressureProfilingHold = 5;
    defaultData.pressureProfilingLength = 15;
    defaultData.pressureProfilingState = true;
    defaultData.preinfusionState = true;
    defaultData.preinfusionSec = 10;
    defaultData.preinfusionBar = 2;
    defaultData.preinfusionSoak = 10;
    defaultData.preinfusionRamp = 0;
    defaultData.preinfusionFlowState = false;
    defaultData.preinfusionFlowVol = 3.5f;
    defaultData.preinfusionFlowTime = 10;
    defaultData.preinfusionFlowSoakTime = 5;
    defaultData.preinfusionFlowPressureTarget = 0;
    defaultData.flowProfileState = false;
    defaultData.flowProfileStart = 3.5f;
    defaultData.flowProfileEnd = 2.0f;
    defaultData.flowProfilePressureTarget = 7;
    defaultData.flowProfileCurveSpeed = 15;
    defaultData.powerLineFrequency = 50;
    defaultData.lcdSleep = 16;
    defaultData.warmupState = false;
    defaultData.homeOnShotFinish = true;
    defaultData.graphBrew = true;
    defaultData.brewDeltaState = true;
    defaultData.switchPhaseOnThreshold = false;
    defaultData.basketPrefill = false;
    defaultData.scalesF1 = 3920;
    defaultData.scalesF2 = 4210;
    defaultData.pumpFlowAtZero = 0.285f;
    defaultData.stopOnWeightState = false;
    defaultData.shotDose = 18.0f;
    defaultData.shotStopOnCustomWeight = 0.f;
    defaultData.shotPreset = 0;

    return defaultData;
  }

  // kind of annoying, but allows reusing macro without messing up type safety
  template <typename T>
  static bool copy_t(T& target, T& source) {
    target = source;
    return true;
  }

  static bool loadCurrentEepromData EEPROM_METADATA_LOADER(EEPROM_DATA_VERSION, eepromMetadata_t, copy_t);

}

bool eepromWrite(eepromValues_t eepromValuesNew) {
  /*
    No need to do these check since it checks UNSIGNED integers

    if (eepromValuesNew.pressureProfilingHold < 0)
    if (eepromValuesNew.pressureProfilingLength < 0)
    if (eepromValuesNew.preinfusionSec < 0)
    if (eepromValuesNew.preinfusionSoak < 0)
    if (eepromValuesNew.preinfusionRamp < 0)
    if (eepromValuesNew.pressureProfilingLength < 0)
    if (eepromValuesNew.offset < 0)
    if (eepromValuesNew.hpwr < 0)
    if (eepromValuesNew.lcdSleep < 0)
  */

  /* THIS IS STILL VERY MUCH UNPROVEN AND GUESSWORK DUE TO ABSENCE OF SOLID EVIDENCE
  Due to flash memory access speeds varying for diffrent operations a theoretical delayMicroseconds(X)
  might be needed in the future. For now this limitation was overcome by spending
  time before a write needs to happen with some additional data integrity checks.
  YOLO
  */
  const char *errMsg = "Data out of range";

  if (eepromValuesNew.preinfusionState != 0 && eepromValuesNew.preinfusionState != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.pressureProfilingState != 0 && eepromValuesNew.pressureProfilingState != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.preinfusionFlowState != 0 && eepromValuesNew.preinfusionFlowState != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

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

  if (eepromValuesNew.flowProfileState != 0 && eepromValuesNew.flowProfileState != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.homeOnShotFinish != 0 && eepromValuesNew.homeOnShotFinish != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.graphBrew != 0 && eepromValuesNew.graphBrew != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.warmupState != 0 && eepromValuesNew.warmupState != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.brewDeltaState != 0 && eepromValuesNew.brewDeltaState != 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.pressureProfilingStart < 1) {
    LOG_ERROR(errMsg);
    return false;
  }

  if (eepromValuesNew.pressureProfilingFinish < 1) {
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

  if (eepromValuesNew.powerLineFrequency != 50 && eepromValuesNew.powerLineFrequency != 60) {
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

  if (version < EEPROM_DATA_VERSION && legacyEepromDataLoaders[version] != NULL) {
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
