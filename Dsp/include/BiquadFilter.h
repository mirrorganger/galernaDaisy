#ifndef EUTERPESND_DSP_BIQUAD_FILTER
#define EUTERPESND_DSP_BIQUAD_FILTER

#include <cmath>
#include <cstdint>
#include <vector>

namespace dsp {

using NumericDataType = float;

class BiquadFilter {
public:
  enum class Type : uint8_t { LOWPASS, HIGHPASS, BANDPASS, LOW_SHELV, HIGH_SHELV};

  struct FilterSettings {
    double samplingFreq;
    double qFactor;
    double cutoffFreq;
    Type filterType;
    uint32_t nChannels;
    double gain_db;
  };

  struct BiquadCoeff {
    double b0, b1, b2, a1, a2;
  };

  BiquadFilter();
  BiquadFilter(const FilterSettings &filterSettings);

  void setUp(const FilterSettings &filterSettings);

  void setQfactor(double qFactor);
  void setType(Type filterType);
  void setCentralFreq(double centralFreq);
  void clear();
  void process(const float* in, float* out, size_t channels, size_t nFrames);

private:
  void reset();
  void update();
  FilterSettings _filterSettings{0};
  std::vector<double> _z1{}, _z2{};
  BiquadCoeff _biquadCoeff;
};

} // namespace dsp

#endif