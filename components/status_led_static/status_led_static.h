#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/status_led/status_led.h"

namespace esphome::status_led_static {

class StatusLEDStatic : public status_led::StatusLED {
 public:
  explicit StatusLEDStatic(GPIOPin *pin);

  void loop() override;
};

}  // namespace esphome::status_led_static
