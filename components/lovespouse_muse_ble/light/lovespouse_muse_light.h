#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "../lovespouse_muse_ble.h"

namespace esphome {
namespace lovespouse_muse_ble {

class LovespouseMuseBleLight : public light::LightOutput, public Component {
 public:
  LovespouseMuseBleLight(LovespouseMuseBleHub *parent) : parent_(parent) {}

  void setup() override;
  void dump_config() override;

  light::LightTraits get_traits() override;
  void write_state(light::LightState *state) override;

 protected:
  LovespouseMuseBleHub *parent_;
};

} // namespace lovespouse_muse_ble
} // namespace esphome
