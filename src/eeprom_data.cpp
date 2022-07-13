#define STM32F4 // This define has to be here otherwise the include of FlashStorage_STM32.h bellow fails.
#include <FlashStorage_STM32.h>
#include "eeprom_data.h"

static struct eepromMetadata_t eepromMetadata;

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

  String errMsg = String("Data out of range");

  if (eepromValuesNew.preinfusionState != 0 && eepromValuesNew.preinfusionState != 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.pressureProfilingState != 0 && eepromValuesNew.pressureProfilingState != 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.homeOnShotFinish != 0 && eepromValuesNew.homeOnShotFinish != 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.graphBrew != 0 && eepromValuesNew.graphBrew != 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.warmupState != 0 && eepromValuesNew.warmupState != 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.brewDeltaState != 0 && eepromValuesNew.brewDeltaState != 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.pressureProfilingStart < 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.pressureProfilingFinish < 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.setpoint < 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.mainDivider < 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.brewDivider < 1) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  if (eepromValuesNew.powerLineFrequency != 50 && eepromValuesNew.powerLineFrequency != 60) {
    LOG_ERROR(errMsg.c_str());
    return false;
  }

  eepromMetadata.timestamp = millis();
  eepromMetadata.version = EEPROM_DATA_VERSION;
  eepromMetadata.values = eepromValuesNew;
  eepromMetadata.versionTimestampXOR = eepromMetadata.timestamp ^ eepromMetadata.version;
  EEPROM.put(0, eepromMetadata);

  return true;
}

eepromValues_t getEepromDefaults(void) {
  eepromValues_t defaultData;

  defaultData.setpoint                       = 100;
  defaultData.offsetTemp                     = 7;
  defaultData.hpwr                           = 550;
  defaultData.mainDivider                    = 5;
  defaultData.brewDivider                    = 2;
  defaultData.pressureProfilingStart         = 9;
  defaultData.pressureProfilingFinish        = 6;
  defaultData.pressureProfilingHold          = 5;
  defaultData.pressureProfilingLength        = 15;
  defaultData.pressureProfilingState         = true;
  defaultData.preinfusionState               = true;
  defaultData.preinfusionSec                 = 10;
  defaultData.preinfusionBar                 = 2;
  defaultData.preinfusionSoak                = 10;
  defaultData.preinfusionRamp                = 0;
  defaultData.powerLineFrequency             = 50;
  defaultData.lcdSleep                       = 16;
  defaultData.warmupState                    = false;
  defaultData.homeOnShotFinish               = true;
  defaultData.graphBrew                      = true;
  defaultData.brewDeltaState                 = true;
  defaultData.scalesF1                       = 4210;
  defaultData.scalesF2                       = 3920;
  defaultData.preinfusionFlowState           = false;
  defaultData.preinfusionFlowVol             = 4.5;
  defaultData.preinfusionFlowTime            = 10;
  defaultData.preinfusionFlowSoakTime        = 5;
  defaultData.preinfusionFlowPressureTarget  = 0;
  defaultData.flowProfileState               = false;
  defaultData.flowProfileStart               = 8.5;
  defaultData.flowProfileEnd                 = 6.0;
  defaultData.flowProfilePressureTarget      = 7;
  defaultData.flowProfileCurveSpeed          = 15;

  return defaultData;
}

void eepromInit(void) {
  EEPROM.get(0, eepromMetadata);
  uint32_t XOR = eepromMetadata.timestamp ^ eepromMetadata.version;

  if (eepromMetadata.version != EEPROM_DATA_VERSION || eepromMetadata.versionTimestampXOR != XOR) {
    LOG_ERROR("SECU_CHECK FAILED! Applying defaults!");
    eepromWrite(getEepromDefaults());
  }
}

struct eepromValues_t eepromGetCurrentValues(void) {
  return eepromMetadata.values;
}
