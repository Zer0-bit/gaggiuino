#ifndef LCD_H
#define LCD_H

#include "eeprom_data.h"

extern volatile int lcdCurrentPageId;
extern volatile int lcdLastCurrentPageId;

void lcdInit(void);
void lcdUploadCfg(eepromValues_t &eepromCurrentValues);
void lcdListen(void);

eepromValues_t lcdDownloadCfg(void);
int lcdGetHomeScreenScalesEnabled(void);
int lcdGetSelectedOperationalMode(void);
int lcdGetManualPressurePower(void);
int lcdGetDescaleCycle(void);

void lcdSetDescaleCycle(int cycle);
void lcdSetPressure(float val);
void lcdSetTemperature(int val);
void lcdSetWeight(float val);
void lcdSetFlow(int val);

void lcdShowDebug(int val1, int val2);
void lcdShowPopup(const char *msg);
void lcdShowDescaleFinished(void);

void lcdBrewTimerStart(void);
void lcdBrewTimerStop(void);
void lcdWarmupStateStop(void);
void lcdWriteLcdMessage(String msg);

/* Triggers */
void lcdTrigger1(void);
void lcdTrigger2(void);
void lcdTrigger3(void);

#endif
