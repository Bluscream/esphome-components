#include "lovespouse_muse_fan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lovespouse_muse_ble {

static const char *TAG = "lovespouse_muse_ble.fan";

void LovespouseMuseBleFan::setup() {
  this->set_supported_preset_modes({"Fast Pulse", "Short Medium", "Ramp Up", "Alternating"});
}

void LovespouseMuseBleFan::dump_config() {
  ESP_LOGCONFIG(TAG, "Lovespouse Muse BLE Fan:");
}

fan::FanTraits LovespouseMuseBleFan::get_traits() {
  auto traits = fan::FanTraits();
  traits.set_speed(true);
  traits.set_supported_speed_count(10);
  this->wire_preset_modes_(traits);
  return traits;
}

void LovespouseMuseBleFan::control(const fan::FanCall &call) {
  if (call.get_state().has_value()) {
    this->state = *call.get_state();
  }
  if (call.get_speed().has_value()) {
    this->speed = *call.get_speed();
  }
  this->apply_preset_mode_(call);
  this->write_state_();
}

void LovespouseMuseBleFan::write_state_() {
  uint8_t raw_cmd = 0x00;
  if (this->state) {
    if (this->has_preset_mode()) {
      auto mode = this->get_preset_mode();
      if (mode == "Fast Pulse") {
        raw_cmd = 0x05;
      } else if (mode == "Short Medium") {
        raw_cmd = 0x04;
      } else if (mode == "Ramp Up") {
        raw_cmd = 0x08;
      } else if (mode == "Alternating") {
        raw_cmd = 0x09;
      }
    } else {
      raw_cmd = 0x10 + this->speed; // Speed 1-10 -> 0x11-0x1A
    }
  }
  this->parent_->send_command(raw_cmd);
  this->publish_state();
}

} // namespace lovespouse_muse_ble
} // namespace esphome

