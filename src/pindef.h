#ifndef PINDEF_H
#define PINDEF_H

// STM32F4 pins definitions
#if defined(SINGLE_BOARD)
#define thermoDO      PB4
#define thermoCS      PA6
#define thermoCLK     PA5

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

#else

#define zcPin PA15
#define thermoDO PA5 //PB4
#define thermoCS PA6 //PB5
#define thermoCLK PA7 //PB6
#define brewPin PA11 // PD7
#define relayPin PB9  // PB0
#define dimmerPin PB3
#define valvePin PC15
#define pressurePin ADS115_A0 //set here just for reference
#define steamPin PA12
#define HX711_sck_1 PB1 //mcu > HX711 no 1 sck pin
#define HX711_sck_2 PB0 //mcu > HX711 no 2 sck pin
#define HX711_dout_1 PA1 //mcu > HX711 no 1 dout pin
#define HX711_dout_2 PA2 //mcu > HX711 no 2 dout pin
#define USART_LCD Serial1
#define USART_DEBUG   Serial  // USB-CDC
#endif
#endif
