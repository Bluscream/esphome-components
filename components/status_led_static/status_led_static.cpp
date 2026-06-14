#include "status_led_static.h"
#include "esphome/core/application.h"

namespace esphome::status_led_static {

StatusLEDStatic::StatusLEDStatic(GPIOPin *pin) : status_led::StatusLED(pin) {
  status_led::global_status_led = this;
}

void StatusLEDStatic::loop() {
  const uint32_t app_state = App.get_app_state();
  if ((app_state & (STATUS_LED_ERROR | STATUS_LED_WARNING)) != 0u) {
    this->pin_->digital_write(true);
  } else {
    this->pin_->digital_write(false);
    this->disable_loop();
  }
}

}  // namespace esphome::status_led_static
