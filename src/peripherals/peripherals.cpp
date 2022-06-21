#include "pindef.h"
#include "peripherals.h"
#include <Arduino.h>

void pinInit() {
  pinMode(relayPin, OUTPUT);
  pinMode(brewPin,  INPUT_PULLUP);
  pinMode(steamPin, INPUT_PULLUP);
  pinMode(HX711_dout_1, INPUT_PULLUP);
  pinMode(HX711_dout_2, INPUT_PULLUP);
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

void closeValve() {
  digitalWrite(valvePin, HIGH);
}

void openValve() {
  digitalWrite(valvePin, LOW);
}
