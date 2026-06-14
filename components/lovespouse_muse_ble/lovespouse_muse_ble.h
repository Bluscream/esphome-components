#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <vector>
#include <string>
#include <cstring>

#ifdef USE_ESP32
#include <esp_gap_ble_api.h>
#include "esphome/components/esp32_ble_server/ble_server.h"
#endif

namespace esphome {
namespace lovespouse_muse_ble {

class LovespouseMuseBleHub : public Component {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;

  void set_device_prefix(const std::string &prefix) { this->device_prefix_ = prefix; }
  void set_device_barcode(uint16_t barcode) { this->device_barcode_ = barcode; }
  void set_device_name(const std::string &name) { this->device_name_ = name; }

  uint16_t get_device_barcode() const { return this->device_barcode_; }
  const std::string &get_device_name() const { return this->device_name_; }

  void add_preset(const std::string &name, uint8_t command) {
    this->presets_.push_back({name, command});
  }
  const std::vector<std::pair<std::string, uint8_t>> &get_presets() const {
    return this->presets_;
  }

  void send_command(uint8_t raw_cmd);

 protected:
  std::string device_prefix_;
  std::string device_name_;
  uint16_t device_barcode_{0};
  std::vector<uint8_t> prefix_bytes_;
  std::vector<std::pair<std::string, uint8_t>> presets_;

  // Whitening and CRC helper functions
  void whitening_init(uint8_t val, uint8_t* ctx);
  uint8_t invert_8(uint8_t value);
  uint16_t invert_16(uint16_t value);
  uint16_t check_crc16(const uint8_t* addr, size_t addrLength, const uint8_t* data, size_t dataLength);
  void whitening_encode(const uint8_t* data, size_t len, uint8_t* ctx, size_t offset, uint8_t* result);
  std::vector<uint8_t> get_rf_payload(const uint8_t* bArr, size_t length, const uint8_t* bArr2, size_t length2);
  uint8_t last_cmd_{0xFF};
  uint32_t last_adv_time_{0};
};

} // namespace lovespouse_muse_ble
} // namespace esphome
