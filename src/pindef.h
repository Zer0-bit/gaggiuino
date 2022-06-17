#ifndef PINDEF_H
#define PINDEF_H

// STM32F4 pins definitions
#define thermoDO      PA5
#define thermoCS      PA6
#define thermoCLK     PA7

#define zcPin         PA15
#define brewPin       PA11
#define relayPin      PB9
#define dimmerPin     PB3
#define steamPin      PA12
#define valvePin      PC15

#define HX711_sck_1   PB0
#define HX711_sck_2   PB1
#define HX711_dout_1  PA1
#define HX711_dout_2  PA2

#define USART_CH      Serial
#define ZC_MODE       RISING

#endif
