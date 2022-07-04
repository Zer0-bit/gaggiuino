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

void scalesInit(float scalesF1, float scalesF2) {

  #if defined(SINGLE_HX711_CLOCK)
    LoadCells.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1);
    LoadCells.set_scale(scalesF1, scalesF2);
    LoadCells.power_up();

    delay(500);

    if (LoadCells.is_ready()) {
      LoadCells.tare(5);
      scalesPresent = true;
    }
  #else
    LoadCell_1.begin(HX711_dout_1, HX711_sck_1);
    LoadCell_2.begin(HX711_dout_2, HX711_sck_2);
    LoadCell_1.set_scale(scalesF1); // calibrated val1
    LoadCell_2.set_scale(scalesF2); // calibrated val2

    delay(500);

    if (LoadCell_1.is_ready() && LoadCell_2.is_ready()) {
      scalesPresent = true;
      LoadCell_1.tare();
      LoadCell_2.tare();
    }
  #endif
}

void scalesTare(void) {
  if(scalesPresent) {
    #if defined(SINGLE_HX711_CLOCK)
      if (LoadCells.is_ready()) LoadCells.tare(5);
    #else
      if (LoadCell_1.wait_ready_timeout(300) && LoadCell_2.wait_ready_timeout(300)) {
        LoadCell_1.tare(2);
        LoadCell_2.tare(2);
      }
    #endif
  }
}

float scalesGetWeight(void) {
  float currentWeight = 0;

  if(scalesPresent) {
    #if defined(SINGLE_HX711_CLOCK)
      if (LoadCells.is_ready()) {
        float values[2];
        LoadCells.get_units(values);
        currentWeight = values[0] + values[1];
      }
    #else
      currentWeight = LoadCell_1.get_units() + LoadCell_2.get_units();
    #endif
  }

  return currentWeight;
}

bool scalesIsPresent(void) {
  return scalesPresent;
}
