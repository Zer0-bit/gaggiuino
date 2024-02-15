/* 09:32 15/03/2023 - change triggering comment */
#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#define DEBOUNCE_DELAY_MS 50

#include "pindef.h"
#include "peripherals.h"
#include <Arduino.h>

// Variables for momentary buttons
#ifdef steamMomentary
static int steamCurrentState = HIGH;
static int steamLastState = HIGH;
static bool steamPressed = false;
static unsigned long steamLastDebounceTime;
#endif
#ifdef brewMomentary
static int brewCurrentState = HIGH;
static int brewLastState = HIGH;
static bool brewPressed = false;
static unsigned long brewLastDebounceTime;
#endif
#ifdef waterMomentary
static int waterCurrentState = HIGH;
static int waterLastState = HIGH;
static bool waterPressed = false;
static unsigned long waterLastDebounceTime;
#endif

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
  #ifdef waterPin
  pinMode(waterPin, INPUT_PULLUP);
  #endif
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

static inline bool momentaryButtonPressed(int pin, int *currentState, int *lastState, bool *buttonIsDown, unsigned long *lastDebounceTime)
{
  int buttonRead = digitalRead(pin);

  if (buttonRead != *lastState) {
      *lastDebounceTime = millis();
  }

  if ((millis() - *lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if (buttonRead != *currentState)
    {
      *currentState = buttonRead;
      if (*currentState == LOW)
      {
        *buttonIsDown = !*buttonIsDown;
      }
    }
  }
  *lastState = buttonRead;
  return *buttonIsDown;
}

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
static inline bool brewState(void) {
  #ifdef brewMomentary
  return momentaryButtonPressed(brewPin, &brewCurrentState, &brewLastState, &brewPressed, &brewLastDebounceTime);
  #else
  return digitalRead(brewPin) == LOW; // pin will be low when switch is ON.
  #endif
}

// Returns HIGH when switch is OFF and LOW when ON
// pin will be high when switch is ON.
static inline bool steamState(void) {
  #ifdef steamMomentary
  return momentaryButtonPressed(steamPin, &steamCurrentState, &steamLastState, &steamPressed, &steamLastDebounceTime);
  #else
  return digitalRead(steamPin) == LOW; // pin will be low when switch is ON.
  #endif
}

static inline bool waterPinState(void) {
  #ifdef waterPin
  #ifdef waterMomentary
  return momentaryButtonPressed(waterPin, &waterCurrentState, &waterLastState, &waterPressed, &waterLastDebounceTime);
  #else
  return digitalRead(waterPin) == LOW; // pin will be low when switch is ON.
  #endif
  #else
  return false;
  #endif
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
