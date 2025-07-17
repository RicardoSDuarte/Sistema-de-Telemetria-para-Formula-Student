#include "LedControl.h"

LedControl::LedControl(int gpioPin) {
  pin = gpioPin;
  state = false;
}

void LedControl::init() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void LedControl::led_on() {
  digitalWrite(pin, LOW);
  state = true;
}

void LedControl::led_off() {
  digitalWrite(pin, HIGH);
  state = false;
}

void LedControl::led_toggle() {
  state = !state;
  digitalWrite(pin, state ? HIGH : LOW);
}
