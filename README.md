# ESPHome Custom Components

A collection of custom ESPHome components by [Bluscream](https://github.com/Bluscream).

## Included Components

1. **`rgb_status_led`**
   * Intelligent status indication using an RGB LED, with connection tracking and OTA progress feedback.
2. **`rgb_status_led_simple`**
   * Simplified RGB status LED wrapper with manual color control and basic light output.
3. **`lovespouse_muse_ble`**
   * Integration for Love Spouse / MuSE BLE device control, offering fan (vibration) and light outputs.
4. **`status_led_static`**
   * A modified version of the standard status LED that stays statically ON during errors or warnings (e.g. Wi-Fi disconnection, API loss) rather than flashing.

## How to Use

To use these components in your ESPHome configuration, add this repository to your `external_components` block:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/Bluscream/esphome-components
    components:
      - rgb_status_led
      - rgb_status_led_simple
      - lovespouse_muse_ble
      - status_led_static
```

---

### Example Usage for `lovespouse_muse_ble`

Defines the BLE hub controller and exposes it as a Home Assistant `fan` entity:

```yaml
lovespouse_muse_ble:
  id: muse_ble
  device_prefix: "wbMSE"
  device_barcode: 8415
  device_name: "AAGS080"

fan:
  - platform: lovespouse_muse_ble
    lovespouse_muse_ble_id: muse_ble
    name: "Toy Vibration"
    id: toy_vibration
    icon: "mdi:vibrate"
```

---

### Example Usage for `rgb_status_led`

Requires float output components (e.g. PWM outputs) for red, green, and blue. Automatically tracks state transitions (WiFi, API, OTA) and blinks or changes color:

```yaml
output:
  - platform: esp32_dac
    pin: GPIO25
    id: red_dac
  - platform: esp32_dac
    pin: GPIO26
    id: green_dac
  - platform: esp32_dac
    pin: GPIO27
    id: blue_dac

light:
  - platform: rgb_status_led
    name: "RGB Status Light"
    red: red_dac
    green: green_dac
    blue: blue_dac
    error_blink_speed: 250ms
    warning_blink_speed: 1500ms
    brightness: 60%
    priority_mode: status
```

---

### Example Usage for `rgb_status_led_simple`

A minimal RGB wrapper that mimics the vanilla `status_led` but supports custom RGB warning and error colors:

```yaml
output:
  - platform: ledc
    pin: GPIO21
    id: red_pwm
  - platform: ledc
    pin: GPIO22
    id: green_pwm
  - platform: ledc
    pin: GPIO23
    id: blue_pwm

light:
  - platform: rgb_status_led_simple
    name: "Simple RGB Status Light"
    red: red_pwm
    green: green_pwm
    blue: blue_pwm
    error_color:
      red: 100%
      green: 0%
      blue: 0%
    warning_color:
      red: 100%
      green: 50%
      blue: 0%
```

---

### Example Usage for `status_led_static`

```yaml
light:
  - platform: status_led_static
    name: "Status LED (Blue)"
    id: status_led_light
    pin: GPIO2
    internal: true
```
