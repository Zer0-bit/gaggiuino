#include "twiControls.h"
#include <Wire.h>

TwoWire twiControlWire(PB3, PB10);

uint8_t ledStateWithParity(const uint8_t);
void twiControlsCheckButtonState(const uint8_t, twiControls);

void twiControlsInit(void) {
  twiControlWire.begin();
}

volatile uint8_t twiControlsButtonState = 0;
volatile uint8_t twiControlsLastButtonState = 0;

void twiControlsRead(SensorState& currentState) {

  twiControlWire.requestFrom(0x00, 1, 0x01, 1, true);

  uint8_t buttonState = twiControlWire.read();

  twiControlsCheckButtonState(buttonState, ONECUP);
  twiControlsCheckButtonState(buttonState, TWOCUP);
  twiControlsCheckButtonState(buttonState, MANUAL);
  twiControlsCheckButtonState(buttonState, WATER);

  //twiControlsCheckButtonState(buttonState, STEAM);

  currentState.brewSwitchState = twiControlsButtonState & MANUAL;
  currentState.steamSwitchState = twiControlsButtonState & STEAM;
  currentState.hotWaterSwitchState = twiControlsButtonState & WATER;

  uint8_t ledState = 0;

  if (currentState.hotWaterSwitchState) {
    currentState.brewSwitchState = false;
    currentState.steamSwitchState = false;
    ledState = WATER;
  }
  if (currentState.steamSwitchState) {
    currentState.brewSwitchState = false;
    currentState.hotWaterSwitchState = false;
    ledState = STEAM;
  }
  if (currentState.brewSwitchState) {
    currentState.steamSwitchState = false;
    currentState.hotWaterSwitchState = false;
    ledState = MANUAL;
  }

  if (currentState.waterLvl < 10) {
    ledState |= ELEMENT;
  }

  twiControlWire.beginTransmission(0x00);

  twiControlWire.write((uint8_t)0x00u);

  twiControlWire.write(ledStateWithParity(ledState));

  twiControlWire.endTransmission();
}

uint8_t ledStateWithParity(const uint8_t ledState) {
  uint8_t result = ledState & 0x3F;
  if ((ledState & 0x01) ^ ((ledState >> 1) & 0x01) ^ ((ledState >> 2) & 0x01)) {
    result |= 0x40;
  }
  if (((ledState >> 3) & 0x01) ^ ((ledState >> 4) & 0x01) ^ ((ledState >> 5) & 0x01)) {
    result |= 0x80;
  }
  return result;
}

void twiControlsCheckButtonState(const uint8_t state, twiControls mask) {
  if (state & mask) {
    twiControlsLastButtonState |= mask;
  }
  else {
    if (twiControlsLastButtonState & mask) {
      if (twiControlsButtonState & mask) {
        twiControlsButtonState &= ~mask;
      }
      else {
        twiControlsButtonState |= mask;
      }
      twiControlsLastButtonState &= ~mask;
    }
  }
}
