// #######################__Options__###########################

#define DEBUG_ENABLED 1                 // Set to 0 if only one Serial available/no debug output desired
#define HEATER_CONTROL 0                // Set to 0 for Time Proportioning (slow PWM), 1 for Phase Angle Control (dimming)
#define BIG_BOILER 0                    // Set to 1 for bigger Boiler (silvia etc)
#define WATCHDOG_ACTIVE 1               // Set to 0 for stm for now
#define HAS_NO_OPV 1                    // Set to 0 if you have an OPV (no matter at what pressure it's set)
#define USE_SECOND_I2C 0                // Set to 1 if you want to use second stm i2c
#define TRANSDUCER_VOLTAGE_0_BAR 0.792  // If transducer is in spec, these should be 0.5...
#define TRANSDUCER_VOLTAGE_12_BAR 4.227 // ... and 4.5 repectively
#define PRESSURE_ON_ADC 1               // Define as 0 to 3, corresponding to A0 to A3 on the ads1115
#define TEMP_ON_ADC 0                   // Define as 0 to 3, corresponding to A0 to A3 on the ads1115
#define SCALES_F1 -3645.07              // Define Scale calibration factors here
#define SCALES_F2 4025.77

#define WATERDISTANCE_MIN 30  // Waterdistance from sensor if Tank is full
#define WATERDISTANCE_MAX 163 // Waterdistance from sensor if Tank is empty

#define WATERDISTANCE_MID ((WATERDISTANCE_MIN + WATERDISTANCE_MAX) / 2)
// ##################__PINOUTS_AND_CONSTANTS__##################

#if defined(ARDUINO_ARCH_AVR)
#define zcPin 2         // Zero Cross detection of dimmer
#define relayPin 48     // Heater SSR Pin
#define pumpPin 4       // Pump dimmer SSR Pin
#define brewPin A2      // Brew switch
#define steamPin A6     // Steam switch
#define solenoidPin 9   // SSR Pin for Brew 3-Way-Solenoid
#define lampPin 11      // LED Indicator light SSR Pin
#define HX711_dout_1 47 // mcu > HX711 no 1 dout pin
#define HX711_dout_2 44 // mcu > HX711 no 2 dout pin
#define HX711_sck_1 42  // mcu > HX711 no 1 sck pin
#define HX711_sck_2 nan // mcu > HX711 no 2 sck pin (not in use for single clock)

#elif defined(ARDUINO_ARCH_STM32) // if arch is stm32
// STM32F4 pins definitions
#define zcPin PA15
#define relayPin PB9 // PB0
#define brewPin PA11 // PD7
#define steamPin PA12
#define lampPin PA3
#define pumpPin PB3
#define solenoidPin PA15 // SSR Pin for Brew 3-Way-Solenoid
#define HX711_sck_1 PB0  // mcu > HX711 no 1 sck pin
#define HX711_dout_1 PA1 // mcu > HX711 no 1 dout pin
#define HX711_dout_2 PA2 // mcu > HX711 no 2 dout pin

#endif

// Define some const values

#define REFRESH_SCREEN_EVERY 150    // Screen refresh interval (ms)
#define DESCALE_PHASE1_EVERY 500    // short pump pulses during descale
#define DESCALE_PHASE2_EVERY 5000   // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY 120000 // long pause for scale softening
#define EEPROM_RESET 1              // change this value if want to reset to defaults
#define PUMP_RANGE 128              // Step amount for Pump control
#define PID_OUTPUT_MAX 1000         // Period Length for one PID Cycle
#define TRIAC_PULSE_MICROS 30       // Triac start pulse length for PAC
#define TRIAC_MIN_DIM 1000          // Min and max values for activation dimmer (PAC)
#define TRIAC_MAX_DIM 9800

#define THERMISTORNOMINAL 100000 // Value of NTC at 25 degrees (should be 100k)
#define TEMPERATURENOMINAL 25    // temp at which NTC has nominal resistance. Hard to measure without accurate equipment, but should almost always be 25 C
#define BCOEFFICIENT 3950        // b-Value specified by manufacturer. Can be measured, but should be close enough
#define SERIESRESISTOR 100000    // Resistor Value used for voltage divider. Can be measured, but 100k is usually close enough

// #############################################################

// Default values

#if DEBUG_ENABLED
#define USART_CH Serial2
#else
#define USART_CH Serial
#endif
