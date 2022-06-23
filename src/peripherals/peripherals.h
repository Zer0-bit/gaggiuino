#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "pindef.h"
#include "peripherals.h"
#include <Arduino.h>

void pinInit(void) {
  pinMode(relayPin, OUTPUT);
  pinMode(brewPin,  INPUT_PULLUP);
  pinMode(steamPin, INPUT_PULLUP);
  pinMode(HX711_dout_1, INPUT_PULLUP);
  pinMode(HX711_dout_2, INPUT_PULLUP);
}

// Actuating the heater element
static inline void setBoilerOn(void) {
  digitalWrite(relayPin, HIGH);  // boilerPin -> HIGH
}

static inline void setBoilerOff(void) {
  digitalWrite(relayPin, LOW);  // boilerPin -> LOW
}

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
static inline bool brewState(void) {  //Monitors the current flowing through the ACS712 circuit and returns a value depending on the power value (P) the system draws
  return digitalRead(brewPin) == LOW; // pin will be low when switch is ON.
}

// Returns HIGH when switch is OFF and LOW when ON
// pin will be high when switch is ON.
static inline bool steamState(void) {
  return digitalRead(steamPin) == LOW; // pin will be low when switch is ON.
}

static inline void openValve(void) {
  digitalWrite(valvePin, HIGH);
}

static inline void closeValve(void) {
  digitalWrite(valvePin, LOW);
}

#endif
