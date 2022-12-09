#include "scales.h"
#include "pindef.h"

#include <HX711_2.h>
HX711_2 LoadCells;

bool scalesPresent;

#if defined SINGLE_HX711_BOARD
  unsigned char scale_clk = OUTPUT;
#else
  unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#endif

void scalesInit(float scalesF1, float scalesF2) {

  LoadCells.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1, HX711_sck_2, 128, scale_clk);
  LoadCells.set_scale(scalesF1, scalesF2);
  LoadCells.power_up();

  if (LoadCells.wait_ready_timeout(700, 20)) {
    LoadCells.tare(4);
    scalesPresent = true;
  }
  #ifdef FORCE_PREDICTIVE_SCALES
  scalesPresent = false;
  #endif
}

void scalesTare(void) {
  if (LoadCells.wait_ready_timeout(100, 20)) {
    LoadCells.tare(4);
  }
}

float scalesGetWeight(void) {
  float currentWeight = 0.f;

  if (LoadCells.wait_ready_timeout(100, 20)) {
    float values[2];
    LoadCells.get_units(values);
    currentWeight = values[0] + values[1];
  }

  return currentWeight;
}

bool scalesIsPresent(void) {
  return scalesPresent;
}

float scalesDripTrayWeight() {
  long value[2];
  LoadCells.read_average(value, 4);

  return ((float)value[0] + (float)value[1]);
}
