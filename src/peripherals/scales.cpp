#include "scales.h"
#include "pindef.h"

#include <HX711_2.h>
namespace {
  class LoadCellSingleton {
  public:
    static HX711_2& getInstance() {
      static HX711_2 instance;
      return instance;
    }
  private:
    LoadCellSingleton() = default;
    ~LoadCellSingleton() = default;
  };
}


bool scalesPresent = {};

#if defined SINGLE_HX711_BOARD
unsigned char scale_clk = OUTPUT;
#else
unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#endif

void scalesInit(float scalesF1, float scalesF2) {
  auto& loadCells = LoadCellSingleton::getInstance();
  loadCells.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1, HX711_sck_2, 128, scale_clk);
  loadCells.set_scale(scalesF1, scalesF2);
  loadCells.power_up();

  if (loadCells.wait_ready_timeout(700, 20)) {
    loadCells.tare(4);
    scalesPresent = true;
  }

  // Forced predicitve scales in case someone with actual hardware scales wants to use them.
  if (FORCE_PREDICTIVE_SCALES) {
    scalesPresent = false;
  }
}

void scalesTare(void) {
  auto& loadCells = LoadCellSingleton::getInstance();
  if (loadCells.wait_ready_timeout(100, 20)) {
    loadCells.tare(4);
  }
}

float scalesGetWeight(void) {
  float currentWeight = 0.f;
  auto& loadCells = LoadCellSingleton::getInstance();
  if (loadCells.wait_ready_timeout(100, 20)) {
    float values[2];
    loadCells.get_units(values);
    currentWeight = values[0] + values[1];
  }

  return currentWeight;
}

bool scalesIsPresent(void) {
  return scalesPresent;
}

float scalesDripTrayWeight() {
  long value[2] = {};
  LoadCellSingleton::getInstance().read_average(value, 4);

  return ((float)value[0] + (float)value[1]);
}
