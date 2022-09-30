#include "scales.h"
#include "pindef.h"

#if defined(SINGLE_HX711_CLOCK)
#include <HX711_2.h>
HX711_2 LoadCells;
#else
#include <HX711.h>
HX711 LoadCell_1; //HX711 1
HX711 LoadCell_2; //HX711 2
#endif

bool scalesPresent;

#ifdef SINGLE_BOARD
  unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#elif defined(SINGLE_BOARD) && defined(SINGLE_HX711_BOARD)
  unsigned char scale_clk = OUTPUT;
#else
  unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#endif

void scalesInit(float scalesF1, float scalesF2) {

  #if defined(SINGLE_HX711_CLOCK)
    LoadCells.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1, HX711_sck_2, 128, scale_clk);
    LoadCells.set_scale(scalesF1, scalesF2);
    LoadCells.power_up();

    if (LoadCells.wait_ready_timeout(700, 50)) {
      LoadCells.tare(4);
      scalesPresent = true;
    }
  #else
    LoadCell_1.begin(HX711_dout_1, HX711_sck_1);
    LoadCell_2.begin(HX711_dout_2, HX711_sck_2);
    LoadCell_1.set_scale(scalesF1); // calibrated val1
    LoadCell_2.set_scale(scalesF2); // calibrated val2

    if (LoadCell_1.wait_ready_timeout(700, 100) && LoadCell_2.wait_ready_timeout(700, 100)) {
      scalesPresent = true;
      LoadCell_1.tare();
      LoadCell_2.tare();
    }
  #endif
  #ifdef FORCE_PREDICTIVE_SCALES
  scalesPresent = false;
  #endif
}

void scalesTare(void) {
  #if defined(SINGLE_HX711_CLOCK)
    if (LoadCells.wait_ready_timeout(700, 100)) {
      LoadCells.tare(4);
    }
  #else
    if (LoadCell_1.wait_ready_timeout(700, 100) && LoadCell_2.wait_ready_timeout(700, 100)) {
      LoadCell_1.tare(2);
      LoadCell_2.tare(2);
    }

  #endif
}

float scalesGetWeight(void) {
  float currentWeight = 0.f;

  #if defined(SINGLE_HX711_CLOCK)
    if (LoadCells.wait_ready_timeout(200, 100)) {
      float values[2];
      LoadCells.get_units(values);
      currentWeight = values[0] + values[1];
    }
  #else
    if (LoadCell_1.wait_ready_timeout(200, 100) && LoadCell_2.wait_ready_timeout(200, 100)) {
      currentWeight = LoadCell_1.get_units() + LoadCell_2.get_units();
    }
  #endif

  return currentWeight;
}

bool scalesIsPresent(void) {
  return scalesPresent;
}

float scalesDripTrayWeight() {
  long value[2];
  #if defined(SINGLE_HX711_CLOCK)
    LoadCells.read_average(value, 4);
  #else
    value[0] = LoadCell_1.read_average(4);
    value[1] = LoadCell_2.read_average(4);
  #endif
  return ((float)value[0] + (float)value[1]);
}
