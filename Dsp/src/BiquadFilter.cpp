
#include "BiquadFilter.h"

#include "Conversions.h"

#include <math.h>

namespace dsp {

/**
 * Computation based on the Transposed Canonical Biquad Form from
 * Designing Audio Effect Plugins in C++. For AAX, AU & VST3 & DSP
 * Theory-Routledge (2019) Chapter 10.22 Other Biquad Structures.
 */
static inline NumericDataType
processInternal(const float &inSample, double &z1, double &z2,
                const BiquadFilter::BiquadCoeff &biquadCoeff) {
  double out = biquadCoeff.b0 * inSample + z1;
  z1 = (inSample * biquadCoeff.b1) - (biquadCoeff.a1 * out) + z2;
  z2 = (inSample * biquadCoeff.b2) - (biquadCoeff.a2 * out);
  return static_cast<float>(out);
}

BiquadFilter::BiquadFilter() {}

BiquadFilter::BiquadFilter(const FilterSettings &filterSettings) {
  setUp(filterSettings);
}

void BiquadFilter::setUp(const FilterSettings &filterSettings) {
  _filterSettings = filterSettings;
  _z1.resize(filterSettings.nChannels);
  _z2.resize(filterSettings.nChannels);
  clear();
  update();
}

void BiquadFilter::setQfactor(double qFactor) {
  _filterSettings.qFactor = qFactor;
  update();
}

void BiquadFilter::setType(Type filterType) {
  _filterSettings.filterType = filterType;
  update();
}

void BiquadFilter::setCentralFreq(double cutoffFreq) {
  _filterSettings.cutoffFreq = cutoffFreq;
  update();
}

void BiquadFilter::clear() {
  for (auto v : {_z1, _z2}) {
    std::fill(v.begin(), v.end(), 0.0);
  }
}

void BiquadFilter::process(const float* in, float* out, size_t nChannels, size_t nFrames) {

  for (size_t sample = 0; sample < nFrames; sample++) {
    size_t sampleOffset = sample * nChannels;
    for (size_t channel = 0; channel < nChannels;
         channel++) {
      float cached = in[sampleOffset + channel];
      out[sampleOffset + channel] =
          processInternal(cached, _z1[channel], _z2[channel], _biquadCoeff);
    }
  }
}

void BiquadFilter::update() {
  switch (_filterSettings.filterType) {
  case Type::LOWPASS: {
    auto k = 1.0 / std::tan(M_PI * _filterSettings.cutoffFreq /
                            _filterSettings.samplingFreq);
    auto kSquared = k * k;
    auto qInv = 1.0 / _filterSettings.qFactor;
    auto norm = 1.0 / (1.0 + k * qInv + kSquared);
    _biquadCoeff.b0 = norm;
    _biquadCoeff.b1 = 2.0 * norm;
    _biquadCoeff.b2 = norm;
    _biquadCoeff.a1 = norm * 2.0 * (1.0 - kSquared);
    _biquadCoeff.a2 = norm * (1.0 - k * qInv + kSquared);
    break;
  }
  case Type::HIGHPASS: {
    auto k = std::tan(M_PI * _filterSettings.cutoffFreq /
                      _filterSettings.samplingFreq);
    auto kSquared = k * k;
    auto qInv = 1.0 / _filterSettings.qFactor;
    auto norm = 1.0 / (1.0 + k * qInv + kSquared);
    _biquadCoeff.b0 = norm;
    _biquadCoeff.b1 = norm * -2.0;
    _biquadCoeff.b2 = norm;
    _biquadCoeff.a1 = norm * 2.0 * (kSquared - 1.0);
    _biquadCoeff.a2 = norm * (1.0 - k * qInv + kSquared);
    break;
  }
  case Type::BANDPASS: {
    auto k = 1.0 / std::tan(M_PI * _filterSettings.cutoffFreq /
                            _filterSettings.samplingFreq);
    auto kSquared = k * k;
    auto qInv = 1.0 / _filterSettings.qFactor;
    auto norm = 1.0 / (1.0 + k * qInv + kSquared);
    _biquadCoeff.b0 = norm * k * qInv;
    _biquadCoeff.b1 = 0.0;
    _biquadCoeff.b2 = -norm * k * qInv;
    _biquadCoeff.a1 = norm * 2.0 * (1.0 - kSquared);
    _biquadCoeff.a2 = norm * (1.0 - k * qInv + kSquared);
    break;
  }
  case Type::LOW_SHELV: {
    /**
     * Coeff values taken from Udo zolzer.
     * DAFX Digital Audio Effects. 2nd edition
     * Table 2.3
     */
    auto K = std::tan(M_PI * _filterSettings.cutoffFreq /
                      _filterSettings.samplingFreq);
    auto K_2 = K * K;
    auto w0 = utilities::fromDecibelsToGain(_filterSettings.gain_db);
    auto sq2 = std::sqrt(2.0);
    auto a0 = 1.0 + sq2 * K + K_2;
    _biquadCoeff.a1 = (2.0 * (K_2 - 1.0)) / a0;
    _biquadCoeff.a2 = (1 - sq2 * K + K_2) / a0;
    _biquadCoeff.b0 = (1 + std::sqrt(2.0 * w0) * K + w0 * K_2) / a0;
    _biquadCoeff.b1 = (2.0 * (w0 * K_2 - 1.0)) / a0;
    _biquadCoeff.b2 = (1 - std::sqrt(2.0 * w0) * K + w0 * K_2) / a0;
    break;
  }
  case Type::HIGH_SHELV: {
    /**
     * Coeff values taken from Udo zolzer.
     * DAFX Digital Audio Effects. 2nd edition
     * Table 2.3
     */
    auto K = std::tan(M_PI * _filterSettings.cutoffFreq /
                      _filterSettings.samplingFreq);
    auto K_2 = K * K;
    auto w0 = utilities::fromDecibelsToGain(_filterSettings.gain_db);
    auto sq2 = std::sqrt(2.0);
    auto a0 = 1.0 + sq2 * K + K_2;
    _biquadCoeff.a1 = (2.0 * (K_2 - 1.0)) / a0;
    _biquadCoeff.a2 = (1 - sq2 * K + K_2) / a0;
    _biquadCoeff.b0 = (w0 + std::sqrt(2.0 * w0) * K + K_2) / a0;
    _biquadCoeff.b1 = (2.0 * (K_2 - w0)) / a0;
    _biquadCoeff.b2 = (w0 - std::sqrt(2.0 * w0) * K + K_2) / a0;
    break;
  }
  default:
    break;
  }
}

} // namespace dsp
