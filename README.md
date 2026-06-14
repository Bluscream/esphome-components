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

### Example Usage for `status_led_static`

```yaml
light:
  - platform: status_led_static
    name: "Status LED (Blue)"
    id: status_led_light
    pin: GPIO2
    internal: true
```
