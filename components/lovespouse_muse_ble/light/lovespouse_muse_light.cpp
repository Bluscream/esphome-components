#include "lovespouse_muse_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lovespouse_muse_ble {

static const char *TAG = "lovespouse_muse_ble.light";

void LovespouseMuseBleLight::setup() {}

void LovespouseMuseBleLight::dump_config() {
  ESP_LOGCONFIG(TAG, "Lovespouse Muse BLE Light");
}

light::LightTraits LovespouseMuseBleLight::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void LovespouseMuseBleLight::write_state(light::LightState *state) {
  float r = 0.0f, g = 0.0f, b = 0.0f;
  state->current_values_as_rgb(&r, &g, &b);

  int ir = (int)(r * 255.0f);
  int ig = (int)(g * 255.0f);
  int ib = (int)(b * 255.0f);

  uint8_t raw_cmd = 0x40; // Default Off

  if (ir > 0 || ig > 0 || ib > 0) {
    struct ColorMapping {
      uint8_t cmd;
      int r, g, b;
    };
    ColorMapping colors[] = {
      {0x47, 255, 0, 0},      // Red (A2 AE 4B)
      {0x46, 255, 165, 0},    // Orange (A3 27 5A)
      {0x49, 144, 238, 144},  // Light Green (AC D0 A2)
      {0x43, 0, 100, 0},      // Dark Green (A6 8A 0D)
      {0x41, 135, 206, 235},  // Sky Blue (A4 98 2E)
      {0x45, 173, 216, 230},  // Light Blue (A0 BC 68)
      {0x42, 0, 0, 255},      // Deep Blue (A7 03 1C)
      {0x44, 216, 191, 216},  // Light Purple (A1 35 79)
      {0x48, 255, 0, 127}     // Rose Red (AD 59 B3)
    };

    uint8_t best_cmd = 0x47;
    int min_dist = 1000000;
    for (auto &c : colors) {
      int dr = ir - c.r;
      int dg = ig - c.g;
      int db = ib - c.b;
      int dist = dr*dr + dg*dg + db*db;
      if (dist < min_dist) {
        min_dist = dist;
        best_cmd = c.cmd;
      }
    }
    raw_cmd = best_cmd;
  }

  this->parent_->send_command(raw_cmd);
}

} // namespace lovespouse_muse_ble
} // namespace esphome
