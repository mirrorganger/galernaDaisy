#pragma once

#include "AllPassFilter.h"
#include "CombFilter.h"

namespace in_detail {

template <typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)> make_array(const T &value,
                                                  std::index_sequence<Is...>) {
  return {{(static_cast<void>(Is), value)...}};
}
} // namespace in_detail

template <std::size_t N, typename T>
constexpr std::array<T, N> make_array(const T &value) {
  return in_detail::make_array(value, std::make_index_sequence<N>());
}

inline float getGainFromReverbTime(float rt60_ms, float delay_ms) {
  return std::pow(10.0, -3.0F * delay_ms / rt60_ms);
}

class SchroederReverb {
public:
  void prepare(float sampleRate_hz, float rt60_ms) {
    _sampleRate_hz = sampleRate_hz;
    for (size_t i = 0; i < _combs.size(); i++) {
      _combs[i].prepare(_sampleRate_hz, _combsDelay_ms[i]);
      _combs[i].setGain(static_cast<float>(
          std::pow(10.0, -3.0F * _combsDelay_ms[i] / rt60_ms)));
    }

    for (size_t i = 0; i < _allPass.size(); i++) {
      _allPass[i].prepare(sampleRate_hz, _allPassDelay_ms[i]);
      _allPass[i].setGain(static_cast<float>(std::sqrt(2.0) / 2.0));
    }

    for (size_t i = 0; i < _preAllPass.size(); i++) {
      _preAllPass[i].prepare(sampleRate_hz, _preAllPassDelay_ms[i]);
      _preAllPass[i].setGain(static_cast<float>(std::sqrt(2.0) / 2.0));
    }
  }

  void setDamping(float damping){
    for(size_t i = 0; i < _combs.size(); ++i){
        _combs[i].setDamping(damping); 
    }
  }

  void setDryWetMix(float dryWetMix){
    _dryWetMix = dryWetMix;
  }

  void process(const float* input, float* output, size_t numberOfChannels,
               size_t framesPerBuffer) {

    for (size_t sample_i = 0; sample_i < framesPerBuffer; ++sample_i) {

      // Stereo to mono conversion.
      float sample = 0.0F;
      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        sample += input[sample_i * numberOfChannels + channel_i];
      }

      sample /= numberOfChannels;

      auto in = sample;
      
      for(size_t i = 0; i < _preAllPass.size(); ++i){
        sample = _preAllPass[i].process(sample); 
      }
      
      auto out = 0.0F;
      for (size_t comb_i = 0U; comb_i < _combs.size(); ++comb_i) {
        auto comb_out = _combs[comb_i].process(sample);
        if (comb_i % 2 == 0)
          comb_out *= -1.0F;
  
        out += comb_out;
      }

      constexpr float factor = 1.0F / static_cast<float>(NUM_COMBS);

      out *= factor;

      for(size_t i = 0; i < _allPass.size(); ++i){
        out = _allPass[i].process(out); 
      }
      

      // if (out < -1.0F)
      //   out = -1.0F;
      // if (out > 1.0F)
      //   out = 1.0F;

      out = out * _dryWetMix+ in * (1.0F - _dryWetMix);

      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        output[sample_i * numberOfChannels + channel_i] = out;
      }
    }

               }

  void process(float *const *signal, size_t numberOfChannels,
               size_t framesPerBuffer) {

    for (size_t sample_i = 0; sample_i < framesPerBuffer; ++sample_i) {

      // Stereo to mono conversion.
      float sample = 0.0;
      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        sample += signal[channel_i][sample_i];
      }

      sample /= numberOfChannels;

      auto in = sample;
      
      for(size_t i = 0; i < _preAllPass.size(); ++i){
        sample = _preAllPass[i].process(sample); 
      }
      
      auto out = 0.0F;
      for (size_t comb_i = 0U; comb_i < _combs.size(); ++comb_i) {
        auto comb_out = _combs[comb_i].process(sample);
        if (comb_i % 2 == 0)
          comb_out *= -1.0F;
  
        out += comb_out;
      }
      out *= (1.0F / static_cast<float>(_combs.size()));

      for(size_t i = 0; i < _allPass.size(); ++i){
        out = _allPass[i].process(out); 
      }
      

      if (out < -1.0F)
        out = -1.0F;
      if (out > 1.0F)
        out = 1.0F;

      out = out * _dryWetMix+ in * (1.0F - _dryWetMix);

      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        signal[channel_i][sample_i] = out;
      }
    }
  }

  void updateReverbTime(float rt60_ms) {
    for (size_t i = 0; i < _combs.size(); i++) {
      _combs[i].setGain(static_cast<float>(
          std::pow(10.0F, -3.0F * _combsDelay_ms[i] / rt60_ms)));
    }
  }

private:
  float _sampleRate_hz = 44100.0F;
  float _dryWetMix = 0.5F;

  constexpr static size_t NUM_COMBS = 12U;
  constexpr static size_t NUM_PRE_ALLPASS = 2U;
  constexpr static size_t NUM_ALLPASS = 8U;

  constexpr static auto MAX_DELAY_IN_SAMPLES = (utilities::power_ceil(static_cast<size_t>(44100.0 * 90.0 / 1000.0)));


  std::array<float, NUM_COMBS> _combsDelay_ms = {29.7F, 30.1F, 32.2F, 34.6F,
                                                 37.2F, 38.1F, 39.7F, 40.5,
                                                 41.3F, 42.6F, 43.7F, 45.6F};

  std::array<float, NUM_ALLPASS> _allPassDelay_ms = {1.0F, 1.5F, 2.3F, 2.9, 3.7F, 4.1F, 4.7F, 5.0F};
  std::array<float, NUM_PRE_ALLPASS> _preAllPassDelay_ms = {1.0f, 2.0F};


  std::array<dsp::CombFilter<MAX_DELAY_IN_SAMPLES>, NUM_COMBS> _combs =
      make_array<NUM_COMBS>(dsp::CombFilter<MAX_DELAY_IN_SAMPLES>(44100.0, 200.0, 0.707F));
  std::array<dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES>, NUM_ALLPASS> _allPass =
      make_array<NUM_ALLPASS>(dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES>(44100.0, 10.0, 0.707F));
  std::array<dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES>, NUM_PRE_ALLPASS> _preAllPass =
      make_array<NUM_PRE_ALLPASS>(dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES>(44100.0, 10.0, 0.707F));
};
