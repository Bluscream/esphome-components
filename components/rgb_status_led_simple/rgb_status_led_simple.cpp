#include "rgb_status_led_simple.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rgb_status_led_simple {

const char *const RGBStatusLEDSimple::TAG = "rgb_status_led_simple";

RGBStatusLEDSimple::RGBStatusLEDSimple() {
  // Initialize with user control by default
  this->user_control_active_ = false;
}

void RGBStatusLEDSimple::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RGB Status LED Simple...");
  
  // Initialize outputs to off
  this->set_rgb_output_(0.0f, 0.0f, 0.0f);
  
  ESP_LOGCONFIG(TAG, "RGB Status LED Simple setup completed");
  ESP_LOGCONFIG(TAG, "  Error blink speed: %ums (matches ESPHome)", this->error_blink_speed_);
  ESP_LOGCONFIG(TAG, "  Warning blink speed: %ums (matches ESPHome)", this->warning_blink_speed_);
  ESP_LOGCONFIG(TAG, "  Brightness: %.1f%%", this->brightness_ * 100.0f);
}

void RGBStatusLEDSimple::dump_config() {
  ESP_LOGCONFIG(TAG, "RGB Status LED Simple:");
  ESP_LOGCONFIG(TAG, "  Error Color: R=%.1f, G=%.1f, B=%.1f", 
                this->error_color_.r * 100.0f, this->error_color_.g * 100.0f, this->error_color_.b * 100.0f);
  ESP_LOGCONFIG(TAG, "  Warning Color: R=%.1f, G=%.1f, B=%.1f", 
                this->warning_color_.r * 100.0f, this->warning_color_.g * 100.0f, this->warning_color_.b * 100.0f);
}

void RGBStatusLEDSimple::loop() {
  // Check if we should show status or user control
  if (this->should_show_status_()) {
    this->apply_status_state_();
  } else {
    // User control - restore the light state
    if (this->light_state_ != nullptr) {
      this->light_state_->current_values_as_brightness(&this->brightness_);
      this->light_state_->current_values_as_rgb(&this->red_output_, &this->green_output_, &this->blue_output_);
    }
  }
}

light::LightTraits RGBStatusLEDSimple::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void RGBStatusLEDSimple::write_state(light::LightState *state) {
  this->light_state_ = state;
  this->user_control_active_ = true;
  
  // Apply the user's desired state immediately if no status is active
  if (!this->should_show_status_()) {
    state->current_values_as_brightness(&this->brightness_);
    state->current_values_as_rgb(&this->red_output_, &this->green_output_, &this->blue_output_);
  }
}

void RGBStatusLEDSimple::set_rgb_output_(const RGBColor &color, float brightness_scale) {
  this->set_rgb_output_(color.r, color.g, color.b, brightness_scale);
}

void RGBStatusLEDSimple::set_rgb_output_(float r, float g, float b, float brightness_scale) {
  float final_brightness = this->brightness_ * brightness_scale;
  
  if (this->red_output_ != nullptr) {
    this->red_output_->set_level(r * final_brightness);
  }
  if (this->green_output_ != nullptr) {
    this->green_output_->set_level(g * final_brightness);
  }
  if (this->blue_output_ != nullptr) {
    this->blue_output_->set_level(b * final_brightness);
  }
}

void RGBStatusLEDSimple::apply_status_state_() {
  uint32_t app_state = App.get_app_state();
  uint32_t now = millis();
  
  // Check for errors (highest priority) - matches ESPHome internal status_led
  if ((app_state & STATUS_LED_ERROR) != 0u) {
    // Fast blinking - match ESPHome timing: 250ms period, 150ms on (60% duty cycle)
    uint32_t period = this->error_blink_speed_;
    uint32_t on_time = period * 3 / 5;  // 60% on, 40% off (matching ESPHome)
    
    if ((now % period) < on_time) {
      if (!this->is_blink_on_) {
        this->set_rgb_output_(this->error_color_);
        this->is_blink_on_ = true;
      }
    } else {
      if (this->is_blink_on_) {
        this->set_rgb_output_(0.0f, 0.0f, 0.0f);
        this->is_blink_on_ = false;
      }
    }
    return;
  }
  
  // Check for warnings - matches ESPHome internal status_led
  if ((app_state & STATUS_LED_WARNING) != 0u) {
    // Slow blinking - match ESPHome timing: 1500ms period, 250ms on (17% duty cycle)
    uint32_t period = this->warning_blink_speed_;
    uint32_t on_time = period / 6;  // 17% on, 83% off (matching ESPHome)
    
    if ((now % period) < on_time) {
      if (!this->is_blink_on_) {
        this->set_rgb_output_(this->warning_color_);
        this->is_blink_on_ = true;
      }
    } else {
      if (this->is_blink_on_) {
        this->set_rgb_output_(0.0f, 0.0f, 0.0f);
        this->is_blink_on_ = false;
      }
    }
    return;
  }
  
  // No status active - turn off LED (user control will restore their color)
  this->set_rgb_output_(0.0f, 0.0f, 0.0f);
  this->is_blink_on_ = false;
}

bool RGBStatusLEDSimple::should_show_status_() {
  // Show status if there are any error or warning flags
  uint32_t app_state = App.get_app_state();
  return ((app_state & STATUS_LED_ERROR) != 0u) || ((app_state & STATUS_LED_WARNING) != 0u);
}

}  // namespace rgb_status_led_simple
}  // namespace esphome
