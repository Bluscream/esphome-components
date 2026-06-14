import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_OUTPUT_ID
from .. import LovespouseMuseBleHub, lovespouse_muse_ble_ns

DEPENDENCIES = ["lovespouse_muse_ble"]


LovespouseMuseBleLight = lovespouse_muse_ble_ns.class_(
    "LovespouseMuseBleLight", light.LightOutput, cg.Component
)

CONF_LOVESPOUSE_MUSE_BLE_ID = "lovespouse_muse_ble_id"

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(LovespouseMuseBleLight),
        cv.GenerateID(CONF_LOVESPOUSE_MUSE_BLE_ID): cv.use_id(LovespouseMuseBleHub),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_LOVESPOUSE_MUSE_BLE_ID])
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], parent)
    await cg.register_component(var, config)
    await light.register_light(var, config)
