#include "lovespouse_muse_ble.h"

namespace esphome {
namespace lovespouse_muse_ble {

static const char *TAG = "lovespouse_muse_ble.hub";

void LovespouseMuseBleHub::setup() {
  if (this->device_prefix_.rfind("wb", 0) == 0) {
    this->prefix_bytes_.push_back(0x77); // 'w'
    this->prefix_bytes_.push_back(0x62); // 'b'
    std::string suffix = this->device_prefix_.substr(2);
    if (suffix.length() == 3) {
      for (char c : suffix) {
        this->prefix_bytes_.push_back((uint8_t)c);
      }
    } else if (suffix.length() == 6) {
      for (size_t i = 0; i < suffix.length(); i += 2) {
        uint8_t byte = (uint8_t) strtol(suffix.substr(i, 2).c_str(), nullptr, 16);
        this->prefix_bytes_.push_back(byte);
      }
    }
  }
}

void LovespouseMuseBleHub::dump_config() {
  ESP_LOGCONFIG(TAG, "Lovespouse Muse BLE Hub:");
  ESP_LOGCONFIG(TAG, "  Device Prefix: %s", this->device_prefix_.c_str());
  ESP_LOGCONFIG(TAG, "  Parsed Prefix Bytes: %s", format_hex_pretty(this->prefix_bytes_).c_str());
}

void LovespouseMuseBleHub::set_advertising_active(bool active) {
  this->advertising_active_ = active;
#ifdef USE_ESP32
  if (!active) {
    esp_ble_gap_stop_advertising();
  }
#endif
}

void LovespouseMuseBleHub::send_command(uint8_t raw_cmd) {
  uint8_t comp_id[2] = {0xF0, 0xFF};
  std::vector<uint8_t> packet(comp_id, comp_id + 2);

  std::vector<uint8_t> rf_payload = this->get_rf_payload(this->prefix_bytes_.data(), this->prefix_bytes_.size(), &raw_cmd, 1);
  for (uint8_t b : rf_payload) {
    packet.push_back(b);
  }

#ifdef USE_ESP32
  if (this->advertising_active_ && esp32_ble_server::global_ble_server != nullptr) {
    ESP_LOGD(TAG, "Broadcasting command %02X using prefix %s: %s", raw_cmd, this->device_prefix_.c_str(), format_hex_pretty(packet).c_str());
    esp32_ble_server::global_ble_server->set_manufacturer_data(packet);

    uint16_t interval_units = 160; // 100ms
    esp_ble_adv_params_t custom_adv_params = {};
    custom_adv_params.adv_int_min = interval_units;
    custom_adv_params.adv_int_max = interval_units;
    custom_adv_params.adv_type = ADV_TYPE_IND;
    custom_adv_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    custom_adv_params.channel_map = ADV_CHNL_ALL;
    custom_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

    this->cancel_timeout("stop_advertising");
    esp_ble_gap_stop_advertising();
    esp_ble_gap_start_advertising(&custom_adv_params);

    // If it is NOT a vibration speed command (0x11-0x1A), stop advertising after 1500ms
    if (raw_cmd < 0x11 || raw_cmd > 0x1A) {
      this->set_timeout("stop_advertising", 1500, []() {
        ESP_LOGD("lovespouse_muse_ble.hub", "Stopping temporary advertising for non-vibration command");
        esp_ble_gap_stop_advertising();
      });
    }
  }
#endif
}

void LovespouseMuseBleHub::whitening_init(uint8_t val, uint8_t* ctx) {
  ctx[0] = 1;
  ctx[1] = (val >> 5) & 1;
  ctx[2] = (val >> 4) & 1;
  ctx[3] = (val >> 3) & 1;
  ctx[4] = (val >> 2) & 1;
  ctx[5] = (val >> 1) & 1;
  ctx[6] = val & 1;
}

uint8_t LovespouseMuseBleHub::invert_8(uint8_t value) {
  uint8_t result = 0;
  for (int i = 0; i < 8; i++) {
    result <<= 1;
    result |= (value & 1);
    value >>= 1;
  }
  return result;
}

uint16_t LovespouseMuseBleHub::invert_16(uint16_t value) {
  uint16_t result = 0;
  for (int i = 0; i < 16; i++) {
    result <<= 1;
    result |= (value & 1);
    value >>= 1;
  }
  return result;
}

uint16_t LovespouseMuseBleHub::check_crc16(const uint8_t* addr, size_t addrLength, const uint8_t* data, size_t dataLength) {
  uint16_t crc = 0xffff;
  for (int i = (int)addrLength - 1; i >= 0; i--) {
    crc ^= addr[i] << 8;
    for (int ii = 0; ii < 8; ii++) {
      if ((crc & 0x8000) != 0) {
        crc = ((crc << 1) ^ 0x1021) & 0xffff;
      } else {
        crc = (crc << 1) & 0xffff;
      }
    }
  }
  for (size_t i = 0; i < dataLength; i++) {
    crc ^= invert_8(data[i]) << 8;
    for (int ii = 0; ii < 8; ii++) {
      if ((crc & 0x8000) != 0) {
        crc = ((crc << 1) ^ 0x1021) & 0xffff;
      } else {
        crc = (crc << 1) & 0xffff;
      }
    }
  }
  crc = (~invert_16(crc)) & 0xffff;
  return crc;
}

void LovespouseMuseBleHub::whitening_encode(const uint8_t* data, size_t len, uint8_t* ctx, size_t offset, uint8_t* result) {
  std::memcpy(result, data, len);
  for (size_t i = 0; i < len; i++) {
    uint8_t var6 = ctx[6];
    uint8_t var5 = ctx[5];
    uint8_t var4 = ctx[4];
    uint8_t var3 = ctx[3];
    uint8_t var52 = var5 ^ ctx[2];
    uint8_t var41 = var4 ^ ctx[1];
    uint8_t var63 = var6 ^ ctx[3];
    uint8_t var630 = var63 ^ ctx[0];

    ctx[0] = (var52 ^ var6) & 1;
    ctx[1] = var630 & 1;
    ctx[2] = var41 & 1;
    ctx[3] = var52 & 1;
    ctx[4] = (var52 ^ var3) & 1;
    ctx[5] = (var630 ^ var4) & 1;
    ctx[6] = (var41 ^ var5) & 1;

    uint8_t c = result[i + offset];
    result[i + offset] = (((c & 0x80) ^ (((var52 ^ var6) & 1) << 7)) |
                          ((c & 0x40) ^ ((var630 & 1) << 6)) |
                          ((c & 0x20) ^ ((var41 & 1) << 5)) |
                          ((c & 0x10) ^ ((var52 & 1) << 4)) |
                          ((c & 0x08) ^ ((var63 & 1) << 3)) |
                          ((c & 0x04) ^ ((var4 & 1) << 2)) |
                          ((c & 0x02) ^ ((var5 & 1) << 1)) |
                          ((c & 0x01) ^ (var6 & 1))) & 0xff;
  }
}

std::vector<uint8_t> LovespouseMuseBleHub::get_rf_payload(const uint8_t* bArr, size_t length, const uint8_t* bArr2, size_t length2) {
  uint8_t ctx_25[7] = {0};
  uint8_t ctx_3F[7] = {0};
  whitening_init(0x25, ctx_25);
  whitening_init(0x3f, ctx_3F);

  size_t length_24 = 0x12 + length + length2;
  size_t length_26 = length_24 + 0x02;

  std::vector<uint8_t> result_25(length_26, 0);
  std::vector<uint8_t> result_3f(length_26, 0);
  std::vector<uint8_t> resultbuf(length_26, 0);

  resultbuf[0x0f] = 0x71;
  resultbuf[0x10] = 0x0f;
  resultbuf[0x11] = 0x55;

  if (length > 0) {
    for (size_t j = 0; j < length; j++) {
      resultbuf[0x0f + 3 + length - j - 1] = bArr[j];
    }
  }
  if (length2 > 0) {
    for (size_t j = 0; j < length2; j++) {
      resultbuf[length_24 - j - 1] = bArr2[j];
    }
  }
  for (size_t i = 0; i < 0x03 + length; i++) {
    resultbuf[0x0f + i] = invert_8(resultbuf[0x0f + i]);
  }

  uint16_t crc16 = check_crc16(bArr, length, bArr2, length2);
  resultbuf[length_24] = crc16 & 0xff;
  resultbuf[length_24 + 1] = (crc16 >> 8) & 0xff;

  whitening_encode(resultbuf.data(), 0x2 + length + length2, ctx_3F, 0x12, result_3f.data());
  whitening_encode(resultbuf.data(), length_26, ctx_25, 0x00, result_25.data());

  for (size_t i = 0; i < length_26; i++) {
    result_25[i] ^= result_3f[i];
  }
  return std::vector<uint8_t>(result_25.begin() + 0x0f, result_25.begin() + 0x0f + 11);
}

} // namespace lovespouse_muse_ble
} // namespace esphome
