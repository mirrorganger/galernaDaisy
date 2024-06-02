#pragma once

#include "Delay.hpp"
#include "OnePoleLpf.h"

namespace dsp {


template<size_t max_delay_samples>
class CombFilter {
public:
  CombFilter(float sampleRate_hz, float delayMax_ms, float gain, bool lpfEnabled = true)
      : _g(gain), _sampleRate_hz(sampleRate_hz),
        _delay(), _lpfEnabled(lpfEnabled){
          _lpf.setFeedbackGain(0.5F);
        }

  void prepare(float sampleRate_hz, float delay_ms) {
    _sampleRate_hz = sampleRate_hz;
    _delay.clear();
    setDelay(delay_ms);
  }

  void setDamping(float damping) {
    _lpf.setFeedbackGain(damping);
  }

  void setGain(float gain) { _g = gain;}

  void setDelay(float delay_ms) {
    _delayIndex = (delay_ms * _sampleRate_hz / 1000.0F);
  }

  inline float process(const float xn) {
    auto yn = _delay[_delayIndex];
    if(_lpfEnabled){
      yn = _lpf.process(yn);      
    }  
    auto delayed = xn + _g * yn;
    _delay.push(delayed);
    return yn;
  }

private:
  float _g;
  float _sampleRate_hz;
  float _delayIndex = 0.0F;
  FractionalStaticDelay<max_delay_samples> _delay;
  bool _lpfEnabled;
  OnePoleLpf _lpf;
};

} // namespace dsp