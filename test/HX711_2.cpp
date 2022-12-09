#include "HX711_2.h"



HX711::HX711() {

}

HX711::~HX711() {

}


void HX711::begin(byte dout, byte pd_sck, byte gain) {

}

bool HX711::is_ready() {
  return true;
}

void HX711::wait_ready(unsigned long delay_ms) {

}

bool HX711::wait_ready_retry(int retries, unsigned long delay_ms) {
  return true;
}

bool HX711::wait_ready_timeout(unsigned long timeout, unsigned long delay_ms) {
  return true;
}

void HX711::set_gain(byte gain) {

}


long HX711::read(unsigned long timeout) {
  return 0;
}

long HX711::read_average(byte times) {
  return 0;
}

long HX711::get_value(byte times) {
  return 0;
}

float HX711::get_units(byte times) {
  return 0;
}

void HX711::tare(byte times) {

}

void HX711::set_scale(float scale) {

}

