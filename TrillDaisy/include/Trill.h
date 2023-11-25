#pragma once

#include "daisy_seed.h"
#include <array>

namespace TrillDaisy {

class Trill {
public:
  using I2cAddress = uint8_t;

  enum class Mode {
    AUTO = -1,
    CENTROID = 0,
    RAW = 1,
    BASELINE = 2,
    DIFF = 3,
    NUM_MODES = 5
  };

  constexpr static std::size_t NUM_DEVICE_TYPES = 4U;

  enum class DeviceType {
    TRILL_NONE = 0,
    TRILL_UNKNOWN = 1,
    TRILL_BAR = 2,
    TRILL_SQUARE = 3,
  };

  struct TrillDefaults 
  {
    Trill::DeviceType device;
    Trill::Mode mode;
    I2cAddress address;
  };

  void init(DeviceType deviceType);

private:
  enum class TRILL_COMMANDS {
    None = 0,
    Mode = 1,
    ScanSettings = 2,
    Prescaler = 3,
    NoiseThreshold = 4,
    Idac = 5,
    BaselineUpdate = 6,
    MinimumSize = 7,
    AutoScanInterval = 16,
    Identify = 255
  };

  void identifyType();
  void sendCommand(uint8_t cmd);
  void sendData(uint8_t const *buff, size_t size);
  bool send(uint8_t *data, uint16_t size);
  bool readData(uint8_t *data, uint16_t size);
  DeviceType _deviceType = DeviceType::TRILL_NONE;
  I2cAddress _deviceAddr;
  Mode _mode;
  daisy::I2CHandle::Config _i2Config;
  daisy::I2CHandle _i2cHandle;
};

} // namespace TrillDaisy
