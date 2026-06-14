#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
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

  void set_device_prefix(const std::string &prefix) { this->device_prefix_ = prefix; }

  void send_command(uint8_t raw_cmd);

 protected:
  std::string device_prefix_;
  std::vector<uint8_t> prefix_bytes_;

  // Whitening and CRC helper functions
  void whitening_init(uint8_t val, uint8_t* ctx);
  uint8_t invert_8(uint8_t value);
  uint16_t invert_16(uint16_t value);
  uint16_t check_crc16(const uint8_t* addr, size_t addrLength, const uint8_t* data, size_t dataLength);
  void whitening_encode(const uint8_t* data, size_t len, uint8_t* ctx, size_t offset, uint8_t* result);
  std::vector<uint8_t> get_rf_payload(const uint8_t* bArr, size_t length, const uint8_t* bArr2, size_t length2);
};

} // namespace lovespouse_muse_ble
} // namespace esphome
