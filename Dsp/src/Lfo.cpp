#include "Lfo.h"

#include <cmath>
namespace dsp {

Lfo::Lfo(Lfo::NumericType samplingFreq_hz, Lfo::NumericType oscFreq_hz,
         Lfo::WaveFormT type)
    : _type(type), _phaseIncrement(oscFreq_hz / samplingFreq_hz) {
}

void Lfo::reset(Lfo::NumericType samplingFreq_hz, Lfo::NumericType oscFreq_hz) {
  _phaseIncrement = oscFreq_hz / samplingFreq_hz;
  _moduloCounter = 0.0;
}

void Lfo::update(Lfo::NumericType samplingFreq_hz,
                 Lfo::NumericType oscFreq_hz) {
  _phaseIncrement = oscFreq_hz / samplingFreq_hz;
}

Lfo::NumericType Lfo::operator()() {
  NumericType out = {};
  switch (_type) {
  case Lfo::WaveFormT::SAW:
    out = static_cast<NumericType>(2.0 * _moduloCounter -
                                   1.0); // unipolar to bipolar
    break;
  case Lfo::WaveFormT::TRIANGLE:
    out = static_cast<NumericType>(2.0 * _moduloCounter -
                                   1.0); // unipolar to bipolar
    out =
        static_cast<NumericType>(2.0 * fabs(out) - 1.0); // bipolar to triangle
    break;
  case Lfo::WaveFormT::SINE:
    out = sinf(_moduloCounter * TWO_PI);
    break;
  default:
    break;
  }
  advanceCounter();
  return out;
}

void Lfo::getBlock(dsp::Lfo::NumericType *const buffer, size_t numSamples) {
  for (size_t sample = 0; sample < numSamples; ++sample) {
    buffer[sample] = (*this)();
  }
}

void Lfo::advanceCounter() {
  _moduloCounter += _phaseIncrement;

  if (_phaseIncrement > 0 && _moduloCounter >= 1.0) {
    _moduloCounter -= NumericType{1.0};
  }

  if (_phaseIncrement < 0 && _moduloCounter <= 0.0) {
    _moduloCounter += NumericType{1.0};
  }
}

} // namespace dsp
