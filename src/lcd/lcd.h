#ifndef LCD_H
#define LCD_H

#include <EasyNextionLibrary.h>
#include "eeprom_data/eeprom_data.h"

extern volatile int lcdCurrentPageId;
extern volatile int lcdLastCurrentPageId;

enum class SCREEN_MODES {
    SCREEN_home,
    SCREEN_brew_settings,
    SCREEN_brew_manual,
    SCREEN_profiles,
    SCREEN_flush,
    SCREEN_descale,
    SCREEN_settings_boiler,
    SCREEN_settings_system,
    SCREEN_brew_graph,
    SCREEN_shot_settings,
    SCREEN_splash
} ;

void lcdInit(void);
void lcdUploadCfg(eepromValues_t &eepromCurrentValues);
void lcdListen(void);

eepromValues_t lcdDownloadCfg(void);
int lcdGetHomeScreenScalesEnabled(void);
int lcdGetSelectedOperationalMode(void);
int lcdGetManualFlowVol(void);
int lcdGetDescaleCycle(void);

void lcdSetDescaleCycle(int cycle);
void lcdSetPressure(float val);
void lcdSetTemperature(int val);
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
void lcdTrigger1(void);
void lcdTrigger2(void);
void lcdTrigger3(void);
void lcdTrigger4(void);

#endif
