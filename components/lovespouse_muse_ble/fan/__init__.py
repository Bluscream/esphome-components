import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan
from esphome.const import CONF_ID
from .. import LovespouseMuseBleHub, lovespouse_muse_ble_ns

DEPENDENCIES = ["lovespouse_muse_ble"]

LovespouseMuseBleFan = lovespouse_muse_ble_ns.class_(
    "LovespouseMuseBleFan", cg.Component, fan.Fan
)

CONF_LOVESPOUSE_MUSE_BLE_ID = "lovespouse_muse_ble_id"

CONFIG_SCHEMA = fan.fan_schema(LovespouseMuseBleFan).extend(
    {
        cv.GenerateID(CONF_LOVESPOUSE_MUSE_BLE_ID): cv.use_id(LovespouseMuseBleHub),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LOVESPOUSE_MUSE_BLE_ID])
    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)
    await fan.register_fan(var, config)
