#ifndef FLASH_STM32_MOCK_H
#define FLASH_STM32_MOCK_H

#include <Arduino.h>
#include <vector>
#include <iostream>

/** The Arduino fake code doesn't expose the STM specific EEPROM methods
 * + getCommitASAP()
 * + setCommitASAP()
 * + get()
 * + put()
 * This class also mimics actually writing data to a vector so it can allow us to perform better functional tests
*/
class EEPROMMockClass {
  std::vector<uint8_t> data;
public:
  EEPROMMockClass() {
    data.resize(5096);
  }

  uint8_t read(int idx) {
    return data[idx];
  }

  void write(int idx, uint8_t val) {
    data[idx] = val;
  }

  void update(int idx, uint8_t val) {
    data[idx] = val;
  }

  uint16_t length() {
    return data.size();
  }

  void commit() {}

  template <typename T>
  T &put(int idx, T &t, int length = sizeof(T)) {
    std::cout << "Going to write " << typeid(T).name() << ":" << length << "bytes to mock EEPROM[" << std::hex << reinterpret_cast<std::uintptr_t>(data.data()) << "]\n";
    memcpy(data.data() + idx, &t, sizeof(T));
    return t;
  }

  template <typename T>
  T &get(int idx, T &t, int length = sizeof(T)) {
    std::cout << "Going to read " << typeid(T).name() << ":" << length << "bytes from EEPROM[" << std::hex << reinterpret_cast<std::uintptr_t>(data.data()) << "]\n";
    memcpy(&t, data.data() + idx, sizeof(T));
    return t;
  }

  void setCommitASAP(bool value = true) {}
  bool getCommitASAP() { return false; }
};

EEPROMMockClass EEPROMMock;

#endif
