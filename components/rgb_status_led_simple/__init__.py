"""
ESPHome RGB Status LED Simple Component

A minimal component that works exactly like ESPHome's internal status_led
but with RGB light support and manual color control when no status is active.

Author: Bluscream
License: MIT
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import CONF_ID, CONF_OUTPUT, CONF_RED, CONF_GREEN, CONF_BLUE
from esphome.core import CoroPriority, coroutine_with_priority

# Component metadata
CODEOWNERS = ["@esphome/core"]
AUTO_LOAD = ["light"]

# Namespace for the component
rgb_status_led_simple_ns = cg.esphome_ns.namespace("rgb_status_led_simple")
RGBStatusLEDSimple = rgb_status_led_simple_ns.class_("RGBStatusLEDSimple", light::LightOutput, cg.Component)

# Configuration keys
CONF_ERROR_COLOR = "error_color"
CONF_WARNING_COLOR = "warning_color"
CONF_ERROR_BLINK_SPEED = "error_blink_speed"
CONF_WARNING_BLINK_SPEED = "warning_blink_speed"
CONF_BRIGHTNESS = "brightness"

# Schema for RGB color configuration
ColorSchema = cv.Schema({
    cv.Required(CONF_RED): cv.percentage,
    cv.Required(CONF_GREEN): cv.percentage,
    cv.Required(CONF_BLUE): cv.percentage,
})

# Main configuration schema - minimal like vanilla status_led
CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        # Component ID for code generation
        cv.GenerateID(): cv.declare_id(RGBStatusLEDSimple),
        
        # Required RGB output connections
        cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
        cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
        cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
        
        # Simple color configuration - only error and warning like vanilla status_led
        cv.Optional(CONF_ERROR_COLOR, default={CONF_RED: 1.0, CONF_GREEN: 0.0, CONF_BLUE: 0.0}): ColorSchema,
        cv.Optional(CONF_WARNING_COLOR, default={CONF_RED: 1.0, CONF_GREEN: 0.5, CONF_BLUE: 0.0}): ColorSchema,
        
        # Timing configuration - matches ESPHome internal status_led exactly
        cv.Optional(CONF_ERROR_BLINK_SPEED, default="250ms"): cv.positive_time_period,
        cv.Optional(CONF_WARNING_BLINK_SPEED, default="1500ms"): cv.positive_time_period,
        cv.Optional(CONF_BRIGHTNESS, default=1.0): cv.percentage,
    }
).extend(cv.COMPONENT_SCHEMA)


@coroutine_with_priority(CoroPriority.STATUS)
async def to_code(config):
    """
    Generate C++ code from the YAML configuration.
    
    This creates a minimal status LED that works exactly like ESPHome's internal
    status_led but with RGB light support.
    """
    # Get the RGB output components
    red = await cg.get_variable(config[CONF_RED])
    green = await cg.get_variable(config[CONF_GREEN])
    blue = await cg.get_variable(config[CONF_BLUE])
    
    # Create the component instance
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await light.register_light(var, config)
    
    # Connect RGB outputs
    cg.add(var.set_red_output(red))
    cg.add(var.set_green_output(green))
    cg.add(var.set_blue_output(blue))
    
    # Configure colors for error and warning states
    error_color = config[CONF_ERROR_COLOR]
    cg.add(var.set_error_color(error_color[CONF_RED], error_color[CONF_GREEN], error_color[CONF_BLUE]))
    
    warning_color = config[CONF_WARNING_COLOR]
    cg.add(var.set_warning_color(warning_color[CONF_RED], warning_color[CONF_GREEN], warning_color[CONF_BLUE]))
    
    # Configure timing - matches ESPHome internal status_led exactly
    cg.add(var.set_error_blink_speed(config[CONF_ERROR_BLINK_SPEED]))
    cg.add(var.set_warning_blink_speed(config[CONF_WARNING_BLINK_SPEED]))
    cg.add(var.set_brightness(config[CONF_BRIGHTNESS]))
    
    # Enable the component in the build
    cg.add_define("USE_RGB_STATUS_LED_SIMPLE")
