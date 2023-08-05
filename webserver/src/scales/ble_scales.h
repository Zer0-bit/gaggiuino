#ifndef BLE_SCALES_H
#define BLE_SCALES_H

#include <string>
#include <vector>

namespace blescales {
  struct Scales {
    std::string name;
    std::string address;
  };

  void init();
  void tare();
  void onWeightReceived(float weight);

  std::vector<Scales> getAvailableScales();
  Scales getConnectedScales();
}

#endif
