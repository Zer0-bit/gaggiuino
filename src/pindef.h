#ifndef PINDEF_H
#define PINDEF_H

// STM32F4 pins definitions
#define thermoDO      PB4
#define thermoCS      PB5
#define thermoCLK     PB3

#define zcPin         PA0
#define brewPin       PC14
#define relayPin      PA15
#define dimmerPin     PA1
#define steamPin      PC15
#define valvePin      PC13

#define HX711_sck_1   PB0
#define HX711_sck_2   PB1
#define HX711_dout_1  PB8
#define HX711_dout_2  PB9

#define USART_LCD     Serial2 // PA2 & PA3
#define USART_DEBUG   Serial  // USB-CDC

#endif
