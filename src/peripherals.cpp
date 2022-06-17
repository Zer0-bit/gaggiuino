#include "ADS1X15.h"
#include "peripherals.h"

ADS1115 ADS(0x48);

void uartInit() {
  USART_CH.begin(115200);
}

void pinInit() {
  pinMode(relayPin, OUTPUT);
  pinMode(brewPin,  INPUT_PULLUP);
  pinMode(steamPin, INPUT_PULLUP);
  digitalWrite(valvePin, LOW);
  pinMode(HX711_dout_1, INPUT_PULLUP);
  pinMode(HX711_dout_2, INPUT_PULLUP);
}

void pressureSensorInit() {
  ADS.begin();
  ADS.setGain(0);      // 6.144 volt
  ADS.setDataRate(7);  // fast
  ADS.setMode(0);      // continuous mode
  ADS.readADC(0);      // first read to trigger
}

float getPressure() {  //returns sensor pressure data
    // 5V/1024 = 1/204.8 (10 bit) or 6553.6 (15 bit)
    // voltageZero = 0.5V --> 102.4(10 bit) or 3276.8 (15 bit)
    // voltageMax = 4.5V --> 921.6 (10 bit) or 29491.2 (15 bit)
    // range 921.6 - 102.4 = 819.2 or 26214.4
    // pressure gauge range 0-1.2MPa - 0-12 bar
    // 1 bar = 68.27 or 2184.5

    return ADS.getValue() / 1706.6f - 1.49f;
}

int8_t getAdsError() {
  return ADS.getError();
}

// Actuating the heater element
void setBoilerOn() {
  digitalWrite(relayPin, HIGH);  // boilerPin -> HIGH
}

void setBoilerOff() {
  digitalWrite(relayPin, LOW);  // boilerPin -> LOW
}

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
bool brewState() {  //Monitors the current flowing through the ACS712 circuit and returns a value depending on the power value (P) the system draws
  return digitalRead(brewPin) == LOW; // pin will be low when switch is ON.
}

// Returns HIGH when switch is OFF and LOW when ON
// pin will be high when switch is ON.
bool steamState() {
  return digitalRead(steamPin) == LOW; // pin will be low when switch is ON.
}
