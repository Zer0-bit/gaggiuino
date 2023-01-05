#include "pressure_sensor.h"
#include "pindef.h"
#include "ADS1X15.h"
#include "../lcd/lcd.h"
#include "i2c_bus_reset.h"

#if defined SINGLE_BOARD
  ADS1015 ADS(0x48);
#else
  ADS1115 ADS(0x48);
#endif

float previousPressure;
float currentPressure;
long adcValue;

void adsInit() {
  ADS.begin();
  ADS.setGain(0);      // 6.144 volt
  ADS.setDataRate(4);  // fast
  ADS.setMode(1);      // single mode
  ADS.readADC(0);      // first read to trigger
}

void pressureReadSensors() {
  if (ADS.isReady()) {
    adcValue = ADS.getValue();
    ADS.readADC(0);
  }
}

float getPressure() {  //returns sensor pressure data
  // voltageZero = 0.5V --> 25.6 (8 bit) or 102.4 (10 bit) or 2666.7 (ADS 15 bit)
  // voltageMax = 4.5V --> 230.4 (8 bit) or 921.6 (10 bit) or 24000 (ADS 15 bit)
  // range 921.6 - 102.4 = 204.8 or 819.2 or 21333.3
  // pressure gauge range 0-1.2MPa - 0-12 bar
  // 1 bar = 17.1 or 68.27 or 1777.8

  // i2cResetState();

  previousPressure = currentPressure;
  #if defined SINGLE_BOARD
    currentPressure = (adcValue - 166) / 111.11f; // 12bit
  #else
    currentPressure = (adcValue - 2666) / 1777.8f; // 16bit
  #endif

  return currentPressure;
}

bool isPressureRaising() {
  return currentPressure > previousPressure + 0.05f;
}

bool isPressureFalling() {
  return currentPressure < previousPressure - 0.05f;
}

bool isPressureFallingFast() {
  return currentPressure < previousPressure - 0.1f;
}

int8_t getAdsError() {
  return ADS.getError();
}

//Serial.print(digitalRead(PIN_SCL));    //should be HIGH
//Serial.println(digitalRead(PIN_SDA));   //should be HIGH, is LOW on stuck I2C bus

void i2cResetState() {
  if(digitalRead(hw_SCL) != HIGH || digitalRead(hw_SDA) != HIGH || !ADS.isConnected()) {
    LOG_INFO("Reset I2C pins");
    short result = I2C_ClearBus(hw_SDA, hw_SCL);
    char tmp[25];
    snprintf(tmp, sizeof(tmp), "I2C error code: %i", result);
    result == 0 ? adsInit() : lcdShowPopup(tmp);
    delay(50);
  }
}
