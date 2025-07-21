#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <Arduino.h>

class LedControl {
  private:
    int pin;
    bool state;

  public:
    LedControl(int gpioPin);
    void init();
    void led_on();
    void led_off();
    void led_toggle();
};

#endif
