#include "Trill.h"

namespace TrillDaisy {

void Trill::init(DeviceType deviceType) {

  static constexpr std::array<TrillDefaults, NUM_DEVICE_TYPES> trillDefaults = {
      TrillDefaults{DeviceType::TRILL_NONE, Mode::AUTO, 0xFF},
      TrillDefaults{DeviceType::TRILL_UNKNOWN, Mode::AUTO, 0xFF},
      TrillDefaults{DeviceType::TRILL_BAR, Mode::CENTROID, 0x20},
      TrillDefaults{DeviceType::TRILL_SQUARE, Mode::CENTROID, 0x28}};

  _deviceAddr = trillDefaults[static_cast<size_t>(deviceType)].address;

  _i2Config.address = _deviceAddr;
  _i2Config.speed = daisy::I2CHandle::Config::Speed::I2C_100KHZ;
  _i2Config.periph = daisy::I2CHandle::Config::Peripheral::I2C_1;
  _i2Config.mode = daisy::I2CHandle::Config::Mode::I2C_MASTER;
  _i2Config.pin_config.scl = {DSY_GPIOB, 8};
  _i2Config.pin_config.sda = {DSY_GPIOB, 9};
  daisy::I2CHandle::Result res = _i2cHandle.Init(_i2Config);
  if (res == daisy::I2CHandle::Result::ERR) {
    // handle
  }
}

void Trill::identifyType() {
  sendCommand(static_cast<uint8_t>(TRILL_COMMANDS::Identify));
}

void Trill::sendCommand(uint8_t cmd) {
  std::array<uint8_t, 2U> buf = {0X00, cmd};
  _i2cHandle.TransmitBlocking(_deviceAddr, buf.data(), buf.size(), 1000);
};

void Trill::sendData(uint8_t const *buff, size_t size) {
  for (size_t i = 0; i < size; i++) {
    std::array<uint8_t, 2U> buf = {0X40, buff[i]};
    _i2cHandle.TransmitBlocking(_deviceAddr, buf.data(), buf.size(), 1000);
  }
};

bool Trill::send(uint8_t *data, uint16_t size) {
  return daisy::I2CHandle::Result::OK !=
         _i2cHandle.TransmitBlocking(_deviceAddr, data, size, 10);
}

bool Trill::readData(uint8_t *data, uint16_t size) {
  return daisy::I2CHandle::Result::OK !=
         _i2cHandle.ReceiveBlocking(_deviceAddr, data, size, 10);
}
} // namespace TrillDaisy