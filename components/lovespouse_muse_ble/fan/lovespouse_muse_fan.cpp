#include "lovespouse_muse_fan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lovespouse_muse_ble {

static const char *TAG = "lovespouse_muse_ble.fan";

void LovespouseMuseBleFan::setup() {
  std::vector<const char *> modes;
  modes.push_back("None");
  for (const auto &preset : this->parent_->get_presets()) {
    modes.push_back(preset.first.c_str());
  }
  this->set_supported_preset_modes(modes);
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
    this->use_speed_mode_ = true;
  }
  this->apply_preset_mode_(call);
  if (call.get_preset_mode() != nullptr) {
    this->use_speed_mode_ = false;
  }
  this->write_state_();
}

void LovespouseMuseBleFan::write_state_() {
  uint8_t raw_cmd = 0x00;
  if (this->state) {
    if (!this->use_speed_mode_ && this->has_preset_mode()) {
      auto mode = this->get_preset_mode();
      bool found = false;
      for (const auto &preset : this->parent_->get_presets()) {
        if (preset.first == mode) {
          raw_cmd = preset.second;
          found = true;
          break;
        }
      }
      if (!found) {
        this->use_speed_mode_ = true;
      }
    }
    if (this->use_speed_mode_) {
      // Speed 1-9 -> 0x11-0x19, Speed 10 -> 0x20
      raw_cmd = (this->speed == 10) ? 0x20 : (0x10 + this->speed);
    }
  }
  this->parent_->send_command(raw_cmd);
  this->publish_state();
}

} // namespace lovespouse_muse_ble
} // namespace esphome
