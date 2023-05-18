/* 09:32 15/03/2023 - change triggering comment */
#ifndef LCD_H
#define LCD_H

#include <EasyNextionLibrary.h>
#include "eeprom_data/eeprom_data.h"

enum class NextionPage: byte {
  /* 00 */ Home,
  /* 01 */ BrewPreinfusion,
  /* 02 */ BrewSoak,
  /* 03 */ BrewProfiling,
  /* 04 */ BrewManual,
  /* 05 */ Flush,
  /* 06 */ Descale,
  /* 07 */ SettingsBoiler,
  /* 08 */ SettingsSystem,
  /* 09 */ BrewGraph,
  /* 0A */ BrewMore,
  /* 0B */ ShotSettings,
  /* 0C */ BrewTransitionProfile,
  /* 0D */ GraphPreview,
  /* 0E */ KeyboardNumeric,
  /* 0F */ KeyboardAlpha
};

extern volatile NextionPage lcdCurrentPageId;
extern volatile NextionPage lcdLastCurrentPageId;

void lcdInit(void);
void lcdUploadProfile(eepromValues_t &eepromCurrentValues);
void lcdUploadCfg(eepromValues_t &eepromCurrentValues);
void uploadPageCfg(eepromValues_t &eepromCurrentValues);
void lcdListen(void);
void lcdWakeUp(void);

void lcdFetchProfileName(eepromValues_t::profile_t &profile, uint8_t index);
void lcdFetchPreinfusion(eepromValues_t::profile_t &profile);
void lcdFetchSoak(eepromValues_t::profile_t &profile);
void lcdFetchBrewProfile(eepromValues_t::profile_t &profile);
void lcdFetchTransitionProfile(eepromValues_t::profile_t &profile);
void lcdFetchDoseSettings(eepromValues_t::profile_t &profile);
void lcdFetchBrewSettings(eepromValues_t &settings);
void lcdFetchTemp(eepromValues_t::profile_t &profile);
void lcdFetchBoiler(eepromValues_t &settings);
void lcdFetchSystem(eepromValues_t &settings);
uint8_t lcdGetSelectedProfile(void);
bool lcdGetPreinfusionFlowState(void);
bool lcdGetProfileFlowState(void);
bool lcdGetTransitionFlowState(void);
int lcdGetHomeScreenScalesEnabled(void);
int lcdGetSelectedOperationalMode(void);
int lcdGetManualFlowVol(void);
int lcdGetDescaleCycle(void);

void lcdSetDescaleCycle(int cycle);
void lcdSetPressure(float val);
void lcdSetTemperature(uint16_t val);
void lcdTargetState(int val);
void lcdSetWeight(float val);
void lcdSetFlow(int val);
void lcdSetUpTime(float val);

void lcdShowDebug(int val1, int val2);
void lcdShowPopup(const char *msg);

void lcdBrewTimerStart(void);
void lcdBrewTimerStop(void);
void lcdSetBrewTimer(int seconds);
void lcdWarmupStateStop(void);

/* Triggers */
void lcdSaveSettingsTrigger(void);
void lcdScalesTareTrigger(void);
void lcdHomeScreenScalesTrigger(void);
void lcdBrewGraphScalesTareTrigger(void);
void lcdRefreshElementsTrigger(void);
void lcdQuickProfileSwitch(void);
void lcdSaveProfileTrigger(void);

#endif
