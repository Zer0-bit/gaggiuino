/* 09:32 15/03/2023 - change triggering comment */
#ifndef LCD_H
#define LCD_H

#include <EasyNextionLibrary.h>
#include "eeprom_data/eeprom_data.h"

extern volatile int lcdCurrentPageId;
extern volatile int lcdLastCurrentPageId;

enum class SCREEN_MODES {
    SCREEN_home,
    SCREEN_brew_preinfusion,
    SCREEN_brew_soak,
    SCREEN_brew_profiling,
    SCREEN_brew_manual,
    SCREEN_flush,
    SCREEN_descale,
    SCREEN_settings_boiler,
    SCREEN_settings_system,
    SCREEN_brew_graph,
    SCREEN_brew_more,
    SCREEN_shot_settings,
    SCREEN_splash,
    SCREEN_graph_preview
} ;

void lcdInit(void);
void lcdUploadProfile(eepromValues_t &eepromCurrentValues);
void lcdUploadCfg(eepromValues_t &eepromCurrentValues);
void uploadPageCfg(eepromValues_t &eepromCurrentValues);
void lcdListen(void);
void lcdWakeUp(void);

eepromValues_t lcdDownloadCfg(bool toSave = false);
int lcdGetSelectedProfile(void);
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
