import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_OUTPUT_ID
from .. import LovespouseMuseBleHub, lovespouse_muse_ble_ns, HUB_LED_SUPPORT

DEPENDENCIES = ["lovespouse_muse_ble"]


LovespouseMuseBleLight = lovespouse_muse_ble_ns.class_(
    "LovespouseMuseBleLight", light.LightOutput, cg.Component
)

CONF_LOVESPOUSE_MUSE_BLE_ID = "lovespouse_muse_ble_id"

def validate_light(config):
    hub_id = config[CONF_LOVESPOUSE_MUSE_BLE_ID]
    hub_id_str = str(hub_id)
    if hub_id_str in HUB_LED_SUPPORT:
        if not HUB_LED_SUPPORT[hub_id_str]:
            raise cv.Invalid(f"Device connected to lovespouse_muse_ble hub '{hub_id_str}' does not support controllable Coloured Lights (LEDs). You cannot configure a light entity for this device.")
    return config

CONFIG_SCHEMA = cv.All(
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(LovespouseMuseBleLight),
            cv.GenerateID(CONF_LOVESPOUSE_MUSE_BLE_ID): cv.use_id(LovespouseMuseBleHub),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    validate_light
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_LOVESPOUSE_MUSE_BLE_ID])
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], parent)
    await cg.register_component(var, config)
    await light.register_light(var, config)
