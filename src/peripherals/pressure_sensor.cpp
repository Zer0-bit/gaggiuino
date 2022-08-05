#include "pressure_sensor.h"
#include "pindef.h"
#include "ADS1X15.h"

float previousPressure;
float currentPressure;
ADS1115 ADS(0x48);

void pressureSensorInit() {
  ADS.begin();
  ADS.setGain(0);      // 6.144 volt
  ADS.setDataRate(7);  // fast
  ADS.setMode(0);      // continuous mode
  ADS.readADC(0);      // first read to trigger
}

float getPressure() {  //returns sensor pressure data
  // voltageZero = 0.5V --> 25.6 (8 bit) or 102.4 (10 bit) or 2666.7 (ADS 15 bit)
  // voltageMax = 4.5V --> 230.4 (8 bit) or 921.6 (10 bit) or 24000 (ADS 15 bit)
  // range 921.6 - 102.4 = 204.8 or 819.2 or 21333.3
  // pressure gauge range 0-1.2MPa - 0-12 bar
  // 1 bar = 17.1 or 68.27 or 1777.8

  previousPressure = currentPressure;
  currentPressure = (ADS.getValue() - 2666) / 1777.8f;
  return currentPressure;
}

bool isPressureFalling() {
  return previousPressure >= currentPressure + 0.03f;
}

int8_t getAdsError() {
  return ADS.getError();
}
