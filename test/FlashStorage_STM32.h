#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>

class EEPROMClass {
public:
  EEPROMClass(void);

  template<typename T>
  T &get(int const address, T &t) {

    return t;
  }

  template<typename T>
  const T &put(int const address, const T &t) {

    return t;
  }

};

extern EEPROMClass EEPROM;

#endif
