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

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LovespouseMuseBleHub),
        cv.Required(CONF_DEVICE_PREFIX):          cv.string,
        cv.Optional(CONF_DEVICE_BARCODE, default=0): cv.positive_int,
        cv.Optional(CONF_DEVICE_NAME,    default=""): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)

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
        data = fetch_json(url)
        if data and isinstance(data, dict) and data.get("response", {}).get("result") is True:
            return data.get("data")

    # Stage 2: Try remote raw GitHub (arz321/MuSe-Protocol master branch)
    if barcode:
        url = f"https://raw.githubusercontent.com/arz321/MuSe-Protocol/master/base/{barcode}.json"
        data = fetch_json(url)
        if data and isinstance(data, dict):
            return data.get("data") if "data" in data else data

    # Stage 3: Try local files cloned with the component (relative to __file__)
    local_db = os.path.join(os.path.dirname(__file__), "devices.json")
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

    # Stage 4 (Fallback Last Resort): Try remote base1000-9999.json from raw github
    url = "https://raw.githubusercontent.com/arz321/MuSe-Protocol/master/base/base1000-9999.json"
    base_data = fetch_json(url)
    if base_data and isinstance(base_data, dict):
        for item in base_data.get("data", []):
            if barcode and item.get("BarCode") == str(barcode):
                return item
            if name and item.get("DeviceTitle") == name:
                return item
            
    return None

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
    
    presets = get_device_presets(config[CONF_DEVICE_BARCODE], config[CONF_DEVICE_NAME])
    for name, command in presets:
        cg.add(var.add_preset(name, command))
