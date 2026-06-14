import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Define namespace
lovespouse_muse_ble_ns = cg.esphome_ns.namespace("lovespouse_muse_ble")
LovespouseMuseBleHub = lovespouse_muse_ble_ns.class_("LovespouseMuseBleHub", cg.Component)

CONF_DEVICE_PREFIX = "device_prefix"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LovespouseMuseBleHub),
        cv.Required(CONF_DEVICE_PREFIX): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_device_prefix(config[CONF_DEVICE_PREFIX]))
