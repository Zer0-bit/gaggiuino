#include "HX711_2.h"



HX711_2::HX711_2() {

}

HX711_2::~HX711_2() {

}


void HX711_2::begin(byte dout, byte dout2, byte pd_sck, byte gain, unsigned char sck_mode) {

}

bool HX711_2::is_ready() {
  return true;
}

void HX711_2::wait_ready(unsigned long delay_ms) {

}

bool HX711_2::wait_ready_retry(int retries, unsigned long delay_ms) {
  return true;
}

bool HX711_2::wait_ready_timeout(unsigned long timeout, unsigned long delay_ms) {
  return true;
}

void HX711_2::set_gain(byte gain) {

}


long HX711_2::read(unsigned long timeout) {
  return 0;
}

void HX711_2::read_average(long *readValues, byte times) {
}

long HX711_2::get_value(byte times) {
  return 0;
}

void HX711_2::get_units(float *readValues, byte times) {
}

void HX711_2::tare(byte times) {

}

void HX711_2::set_scale(float scale, float scale2) {

}

void HX711_2::power_down() {

};

void HX711_2::power_up() {

};
