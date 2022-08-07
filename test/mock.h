#ifndef MOCKS_H
#define MOCKS_H

#define PA0   1
#define PA4   1
#define PA5   1
#define PA6   1
#define PA11  1
#define PA12  1
#define PA15  1
#define PC13  1
#define PC14  1
#define PC15  1
#define PA1   1
#define PB0   1
#define PB1   1
#define PB3   1
#define PB4   1
#define PB8   1
#define PB9   1

#define Serial1   1
#define Serial2   1
#define AUTO_VERSION "test"

#define va_start(args, msg)
#define va_end(args)

struct eepromValues_t {
    uint16_t setpoint;
    uint16_t offsetTemp;
    uint16_t hpwr;
    uint16_t mainDivider;
    uint16_t brewDivider;
};
eepromValues_t runningCfg;

struct SensorState {
  float temperature;
  float pressure;
};
SensorState currentState;

bool preinfusionFinished;
bool brewActive;

static void lcdTrigger1(void) {}
static void lcdTrigger2(void) {}
static void lcdTrigger3(void) {}
#endif
