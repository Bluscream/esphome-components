#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"
#include "../lovespouse_muse_ble.h"

namespace esphome {
namespace lovespouse_muse_ble {

class LovespouseMuseBleFan : public fan::Fan, public Component {
 public:
  LovespouseMuseBleFan(LovespouseMuseBleHub *parent) : parent_(parent) {}

  void setup() override;
  void dump_config() override;

  fan::FanTraits get_traits() override;
  void control(const fan::FanCall &call) override;

 protected:
  void write_state_();

  LovespouseMuseBleHub *parent_;
};

} // namespace lovespouse_muse_ble
} // namespace esphome
