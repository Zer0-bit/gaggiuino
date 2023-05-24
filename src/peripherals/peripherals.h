/* 09:32 15/03/2023 - change triggering comment */
#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "pindef.h"
#include "peripherals.h"
#include <Arduino.h>

static inline void pinInit(void) {
  #if defined(LEGO_VALVE_RELAY)
    pinMode(valvePin, OUTPUT_OPEN_DRAIN);
  #else
    pinMode(valvePin, OUTPUT);
  #endif
  pinMode(relayPin, OUTPUT);
  #ifdef steamValveRelayPin
  pinMode(steamValveRelayPin, OUTPUT);
  #endif
  #ifdef steamBoilerRelayPin
  pinMode(steamBoilerRelayPin, OUTPUT);
  #endif
  pinMode(brewPin,  INPUT_PULLUP);
  pinMode(steamPin, INPUT_PULLUP);
  pinMode(waterPin, INPUT_PULLUP);
}

// Actuating the heater element
static inline void setBoilerOn(void) {
  digitalWrite(relayPin, HIGH);  // boilerPin -> HIGH
}

static inline void setBoilerOff(void) {
  digitalWrite(relayPin, LOW);  // boilerPin -> LOW
}

static inline void setSteamValveRelayOn(void) {
  #ifdef steamValveRelayPin
  digitalWrite(steamValveRelayPin, HIGH);  // steamValveRelayPin -> HIGH
  #endif
}

static inline void setSteamValveRelayOff(void) {
  #ifdef steamValveRelayPin
  digitalWrite(steamValveRelayPin, LOW);  // steamValveRelayPin -> LOW
  #endif
}

static inline void setSteamBoilerRelayOn(void) {
  #ifdef steamBoilerRelayPin
  digitalWrite(steamBoilerRelayPin, HIGH);  // steamBoilerRelayPin -> HIGH
  #endif
}

static inline void setSteamBoilerRelayOff(void) {
  #ifdef steamBoilerRelayPin
  digitalWrite(steamBoilerRelayPin, LOW);  // steamBoilerRelayPin -> LOW
  #endif
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

static inline bool waterPinState(void) {
  return digitalRead(waterPin) == LOW; // pin will be low when switch is ON.
}

static inline void openValve(void) {
  #if defined LEGO_VALVE_RELAY
    digitalWrite(valvePin, LOW);
  #else
    digitalWrite(valvePin, HIGH);
  #endif
}

static inline void closeValve(void) {
  #if defined LEGO_VALVE_RELAY
    digitalWrite(valvePin, HIGH);
  #else
    digitalWrite(valvePin, LOW);
  #endif
}

#endif
