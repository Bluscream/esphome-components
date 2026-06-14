#include "lovespouse_muse_fan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lovespouse_muse_ble {

static const char *TAG = "lovespouse_muse_ble.fan";

void LovespouseMuseBleFan::setup() {
  this->set_supported_preset_modes({"None", "Preset 1", "Preset 2", "Preset 3", "Short Medium", "Fast Pulse", "Preset 6", "Preset 7", "Ramp Up", "Alternating"});
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
    if (this->has_preset_mode() && this->get_preset_mode() != "None") {
      auto mode = this->get_preset_mode();
      if (mode == "Preset 1") {
        raw_cmd = 0x01;
      } else if (mode == "Preset 2") {
        raw_cmd = 0x02;
      } else if (mode == "Preset 3") {
        raw_cmd = 0x03;
      } else if (mode == "Short Medium") {
        raw_cmd = 0x04;
      } else if (mode == "Fast Pulse") {
        raw_cmd = 0x05;
      } else if (mode == "Preset 6") {
        raw_cmd = 0x06;
      } else if (mode == "Preset 7") {
        raw_cmd = 0x07;
      } else if (mode == "Ramp Up") {
        raw_cmd = 0x08;
      } else if (mode == "Alternating") {
        raw_cmd = 0x09;
      }
    } else {
      raw_cmd = (this->speed == 10) ? 0x20 : (0x10 + this->speed); // Speed 1-9 -> 0x11-0x19, Speed 10 -> 0x20
    }
  }
  this->parent_->send_command(raw_cmd);
  this->publish_state();
}

} // namespace lovespouse_muse_ble
} // namespace esphome

