from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PIN
from esphome.core import CoroPriority, coroutine_with_priority

status_led_static_ns = cg.esphome_ns.namespace("status_led_static")
StatusLEDStatic = status_led_static_ns.class_("StatusLEDStatic", cg.Component)

AUTO_LOAD = ["status_led"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(StatusLEDStatic),
        cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


@coroutine_with_priority(CoroPriority.STATUS)
async def to_code(config):
    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    rhs = StatusLEDStatic.new(pin)
    var = cg.Pvariable(config[CONF_ID], rhs)
    await cg.register_component(var, config)
    cg.add(var.pre_setup())
    cg.add_define("USE_STATUS_LED")
