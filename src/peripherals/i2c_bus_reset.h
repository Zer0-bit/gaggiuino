/* 09:32 15/03/2023 - change triggering comment */
#ifndef I2C_BUS_RESET_H
#define I2C_BUS_RESET_H

#include <Arduino.h>
#include <Wire.h>

/**
* This routine turns off the I2C bus and clears it
* on return scaPin and sclPin pins are tri-state inputs with PULLUPS
* You need to call Wire.begin() after this to re-enable I2C
* This routine does NOT use the Wire library at all.
*
* returns 0 if bus cleared
*         1 if sclPin held low.
*         2 if sdaPin held low by slave clock stretch for > 2sec
*         3 if sdaPin held low after 20 clocks.
*/
int I2C_ClearBus(int sdaPin, int sclPin) {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the sdaPin and sclPin pins directly
#endif
  pinMode(sdaPin, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins Inputs with pullup.
  pinMode(sclPin, INPUT_PULLUP);

  bool SCL_LOW = (digitalRead(sclPin) == LOW); // Check is sclPin is Low.
  if (SCL_LOW) { //If it is held low MCU cannot become the I2C master.
    pinMode(sdaPin, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins Inputs with pullup.
    pinMode(sclPin, INPUT_PULLUP);
    return 1; // I2C bus error. Could not clear sclPin clock line held low
  }

  bool SDA_LOW = (digitalRead(sdaPin) == LOW);  // vi. Check sdaPin input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If sdaPin is Low,
    clockCount--;
    // Note: I2C bus is open collector so do NOT drive sclPin or sdaPin high.
    pinMode(sclPin, INPUT); // release sclPin pullup so that when made output it will be LOW
    pinMode(sclPin, OUTPUT); // then clock sclPin Low
    delayMicroseconds(10); //  for >5us
    pinMode(sclPin, INPUT); // release sclPin LOW
    pinMode(sclPin, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5us
    // The >5us is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(sclPin) == LOW); // Check if sclPin is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for sclPin to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(sclPin) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      pinMode(sdaPin, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins Inputs with pullup.
      pinMode(sclPin, INPUT_PULLUP);
      return 2; // I2C bus error. Could not clear. sclPin clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(sdaPin) == LOW); //   and check sdaPin input again and loop
  }
  if (SDA_LOW) { // still low
    pinMode(sdaPin, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins Inputs with pullup.
    pinMode(sclPin, INPUT_PULLUP);
    return 3; // I2C bus error. Could not clear. sdaPin data line held low
  }

  // else pull sdaPin line low for Start or Repeated Start
  pinMode(sdaPin, INPUT); // remove pullup.
  pinMode(sdaPin, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5us
  pinMode(sdaPin, INPUT); // remove output low
  pinMode(sdaPin, INPUT_PULLUP); // and make sdaPin high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5us
  pinMode(sdaPin, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins Inputs with pullup.
  pinMode(sclPin, INPUT_PULLUP);
  return 0; // all ok
}

#endif
