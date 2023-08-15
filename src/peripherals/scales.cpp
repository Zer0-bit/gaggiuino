/* 09:32 15/03/2023 - change triggering comment */
#include "scales.h"
#include "pindef.h"
#include "remote_scales.h"
#include "gaggia_settings.h"

#include <HX711_2.h>
namespace {
  class LoadCellSingleton {
  public:
    static HX711_2& getInstance() {
      static HX711_2 instance(TIM3);
      return instance;
    }
  private:
    LoadCellSingleton() = default;
    ~LoadCellSingleton() = default;
  };
}

ScalesSettings currentScalesSettings;

bool hwScalesPresent = false;

#if defined SINGLE_HX711_BOARD
unsigned char scale_clk = OUTPUT;
#else
unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#endif

void scalesInit(const ScalesSettings& settings) {
  currentScalesSettings = settings;
  hwScalesPresent = false;
  // Forced predicitve scales in case someone with actual hardware scales wants to use them.
  if (currentScalesSettings.forcePredictive) {
    return;
  }

  if (currentScalesSettings.hwScalesEnabled) {
    auto& loadCells = LoadCellSingleton::getInstance();
    loadCells.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1, 128U, scale_clk);
    loadCells.set_scale(currentScalesSettings.hwScalesF1, currentScalesSettings.hwScalesF2);
    loadCells.power_up();

    if (loadCells.wait_ready_timeout(1000, 10)) {
      loadCells.tare(4);
      hwScalesPresent = true;
  }
    else {
      loadCells.power_down();
    }
}

  if (!hwScalesPresent && settings.btScalesEnabled && remoteScalesIsPresent()) {
    remoteScalesTare();
  }
}

void scalesTare(void) {
  if (currentScalesSettings.hwScalesEnabled && hwScalesPresent) {
    auto& loadCells = LoadCellSingleton::getInstance();
    if (loadCells.wait_ready_timeout(150, 10)) {
      loadCells.tare(4);
    }
  }
  else if (currentScalesSettings.btScalesEnabled && remoteScalesIsPresent()) {
    remoteScalesTare();
  }
}

Measurement scalesGetWeight(void) {
  Measurement currentWeight = Measurement{ .value = 0.f, .millis = 0 };
  if (hwScalesPresent) {
    auto& loadCells = LoadCellSingleton::getInstance();
    if (loadCells.wait_ready_timeout(150, 10)) {
      float values[2];
      loadCells.get_units(values);
      currentWeight = Measurement{ .value = values[0] + values[1], .millis = static_cast<uint32_t>(millis()) };
    }
  }
  else if (remoteScalesIsPresent()) {
    currentWeight = remoteScalesGetWeight();
  }
  currentWeight.setPrecision(1);
  return currentWeight;
}

bool scalesIsPresent(void) {
  // Forced predicitve scales in case someone with actual hardware scales wants to use them.
  if (currentScalesSettings.forcePredictive) {
    return false;
  }
  return (currentScalesSettings.hwScalesEnabled && hwScalesPresent) ||
    (currentScalesSettings.btScalesEnabled && remoteScalesIsPresent());
}

float scalesDripTrayWeight() {
  long value[2] = {};
  if (hwScalesPresent) {
    LoadCellSingleton::getInstance().read_average(value, 4);
  }
  return ((float)value[0] + (float)value[1]);
}

void scalesCalibrate(void) {
  static unsigned long timer = millis();

  if (currentScalesSettings.hwScalesEnabled && hwScalesPresent) {
    float values[2];
    static float previousFactor1, previousFactor2;
    auto& loadCells = LoadCellSingleton::getInstance();

    if (currentScalesSettings.hwScalesF1 != previousFactor1 || currentScalesSettings.hwScalesF1 != previousFactor2) {
      loadCells.set_scale(currentScalesSettings.hwScalesF1, currentScalesSettings.hwScalesF2);
      previousFactor1 = currentScalesSettings.hwScalesF1;
      previousFactor2 = currentScalesSettings.hwScalesF2;
    }

    if (millis() > timer) {
      loadCells.get_units(values);
      // write vals to the weight boxes
      timer = millis() + 100ul;
    }
  }

}
