#ifndef LED_H
#define LED_H

#include <Arduino.h>

#include <NCP5623.h>

void ledInit(void);
void ledColor(uint8_t red, uint8_t green, uint8_t blue);

#endif
