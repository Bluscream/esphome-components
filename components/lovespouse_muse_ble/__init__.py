import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

import os
import json

# Define namespace
lovespouse_muse_ble_ns = cg.esphome_ns.namespace("lovespouse_muse_ble")
LovespouseMuseBleHub = lovespouse_muse_ble_ns.class_("LovespouseMuseBleHub", cg.Component)

CONF_DEVICE_PREFIX  = "device_prefix"
CONF_DEVICE_BARCODE = "device_barcode"
CONF_DEVICE_NAME    = "device_name"

import urllib.request
import urllib.error

def fetch_json(url):
    try:
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'Mozilla/5.0 (ESPHome component)'}
        )
        with urllib.request.urlopen(req, timeout=5) as response:
            return json.loads(response.read().decode("utf-8"))
    except Exception:
        return None

def find_device_profile(barcode, name):
    # Stage 1: Try official static API URL (does not require login/auth)
    if barcode:
        url = f"https://lovespouse.zlmicro.com/index.php?g=App&m=Diyapp&a=getproductdetail&barcode={barcode}&userid=-1"
        # Alternative official API (requires active auth token header):
        # https://api.peacelovebd.com/muse/v1/device/product_detail?barcode={barcode}
        data = fetch_json(url)
        if data and isinstance(data, dict) and data.get("response", {}).get("result") is True:
            return data.get("data")

    # Stage 2: Try local consolidated file cloned with the component (relative to __file__)
    local_db = os.path.join(os.path.dirname(__file__), "devices.json")
    # Alternative remote fallbacks previously used (removed to optimize compile latency):
    # - Single device file: https://raw.githubusercontent.com/arz321/MuSe-Protocol/master/base/{barcode}.json
    # - Base database index: https://raw.githubusercontent.com/arz321/MuSe-Protocol/master/base/base1000-9999.json
    if os.path.exists(local_db):
        try:
            with open(local_db, "r", encoding="utf-8") as f:
                base_data = json.load(f)
                for item in base_data.get("data", []):
                    if barcode and item.get("BarCode") == str(barcode):
                        return item
                    if name and item.get("DeviceTitle") == name:
                        return item
        except Exception:
            pass
            
    return None

def validate_hub(config):
    prefix = config.get(CONF_DEVICE_PREFIX)
    barcode = config.get(CONF_DEVICE_BARCODE)
    name = config.get(CONF_DEVICE_NAME)
    if not prefix:
        if not barcode and not name:
            raise cv.Invalid("Either device_prefix, device_barcode, or device_name must be specified.")
        profile = find_device_profile(barcode, name)
        if profile and "BroadcastPrefix" in profile:
            raw_prefix = profile["BroadcastPrefix"]
            try:
                resolved = "".join([chr(int(x, 16)) for x in raw_prefix.split()])
                config[CONF_DEVICE_PREFIX] = resolved
            except Exception as e:
                raise cv.Invalid(f"Failed to parse BroadcastPrefix '{raw_prefix}' from device profile: {e}")
        else:
            raise cv.Invalid("device_prefix is required but could not be resolved from device_barcode or device_name.")
    return config

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LovespouseMuseBleHub),
            cv.Optional(CONF_DEVICE_PREFIX):          cv.string,
            cv.Optional(CONF_DEVICE_BARCODE, default=0): cv.positive_int,
            cv.Optional(CONF_DEVICE_NAME,    default=""): cv.string,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    validate_hub
)

def get_device_presets(barcode, name):
    profile = find_device_profile(barcode, name)
    
    classic_presets = [
        ("Speed 1", 0x01),
        ("Speed 2", 0x02),
        ("Speed 3", 0x03),
        ("4x Short 1x Medium", 0x04),
        ("Short Pulse Ramp", 0x05),
        ("Medium Fast Slow", 0x06),
        ("9x Short 1x Long", 0x07),
        ("Ramp", 0x08),
        ("Alternating", 0x09),
    ]
    
    if profile is not None:
        classic_ids = profile.get("ClassicId", [])
        if classic_ids and isinstance(classic_ids, list):
            classic_info = classic_ids[0]
            if classic_info.get("Id") == 1 or not classic_info.get("Groups"):
                return classic_presets
            
            groups = classic_info.get("Groups", [])
            presets = []
            for g in groups:
                g_name = g.get("Name", "")
                buttons = g.get("Buttons", [])
                for b in buttons:
                    b_name = b.get("Name", "")
                    cmd = b.get("Command")
                    if cmd is not None:
                        if len(groups) > 1:
                            presets.append((f"{g_name} {b_name}", int(cmd)))
                        else:
                            presets.append((b_name, int(cmd)))
            if presets:
                return presets
        return classic_presets
        
    default_presets = list(classic_presets)
    for i in range(1, 10):
        default_presets.append((f"Wave {i}", 0x30 + i))
    for i in range(1, 10):
        default_presets.append((f"Vibe {i}", 0x40 + i))
    for i in range(1, 6):
        default_presets.append((f"Frequency {i}", 0x50 + i))
    return default_presets

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_device_prefix(config[CONF_DEVICE_PREFIX]))
    cg.add(var.set_device_barcode(config[CONF_DEVICE_BARCODE]))
    cg.add(var.set_device_name(config[CONF_DEVICE_NAME]))
    
    # Check if the device profile has controllable LEDs (Coloured Lights)
    profile = find_device_profile(config[CONF_DEVICE_BARCODE], config[CONF_DEVICE_NAME])
    has_led = False
    if profile:
        classic_ids = profile.get("ClassicId", [])
        if classic_ids and isinstance(classic_ids, list):
            for classic_info in classic_ids:
                groups = classic_info.get("Groups", [])
                for g in groups:
                    g_name = g.get("Name", "")
                    if "Coloured Lights" in g_name or "发光" in g_name:
                        has_led = True
                        break
        title = profile.get("DeviceTitle") or config[CONF_DEVICE_NAME] or str(config[CONF_DEVICE_BARCODE])
        if has_led:
            print(f"INFO: [lovespouse_muse_ble] Device '{title}' supports controllable Coloured Lights (LEDs).")
        else:
            print(f"INFO: [lovespouse_muse_ble] Device '{title}' does not support controllable Coloured Lights (LEDs).")
    
    presets = get_device_presets(config[CONF_DEVICE_BARCODE], config[CONF_DEVICE_NAME])
    for name, command in presets:
        cg.add(var.add_preset(name, command))
