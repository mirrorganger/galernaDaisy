#pragma once

#include "AllPassFilter.h"
#include "CombFilter.h"
#include "Delay.hpp"
#include "Lfo.h"
#include "OnePoleLpf.h"

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

struct InputDiffuser {

  void reset() {}

  void prepare(float sampleRate_hz) {

    for (size_t i = 0; i < _allPass.size(); i++) {
      float delay_ms =
          static_cast<float>(_allPassDelaySamples[i]) * 1000.0F / sampleRate_hz;
      _allPass[i].prepare(sampleRate_hz, delay_ms);
      _allPass[i].setGain(_diffusionGains[i]);
    }
  }

  inline float process(float in) {
    for(auto & allPass : _allPass){
      in = allPass.process(in);
    }
    return in;
  }

  constexpr static size_t NUM_ALLPASS = 4;
  std::array<float, NUM_ALLPASS> _diffusionGains = {0.750F, 0.750F, 0.625F,
                                                    0.625F};
  std::array<size_t, NUM_ALLPASS> _allPassDelaySamples = {210U, 158U, 561U,
                                                          410U};
  constexpr static float MAX_DELAY_MS = 10.0;
  constexpr static auto MAX_DELAY_IN_SAMPLES = (utilities::power_ceil(static_cast<size_t>(44100.0 * MAX_DELAY_MS / 1000.0)));
  
  std::array<dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES>, NUM_ALLPASS> _allPass =
      make_array<NUM_ALLPASS>(dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES>(44100.0, 10.0, 0.707F));
};

struct TankBranch {
  struct Configuration {
    float lfoMaxExcursion;
    std::array<float, 2U> delays;
    // For modulated allpass and allpass
    std::array<float, 2U> diffusionDecays;
    std::array<float, 2U> diffusionDelays;
    std::array<float, 7U> outputTaps;
  };

  void prepare(float sampleRate_hz, Configuration config) {
    _config = config;

    for (auto &delay : _delays) {
      delay.clear();
    }

    // prepare lfo
    _lfo.update(sampleRate_hz, 0.1F);
    // prepare modulated all pass
    _modAllPass.prepare(sampleRate_hz,
                        config.diffusionDelays[0] * sampleRate_hz / 1000.0F);
    _modAllPass.setGain(config.diffusionDecays[0]);
    // prepare damping filter
    _dampingFilter.setFeedbackGain(_damping);
    // prepare all passes.
    _allPass.prepare(sampleRate_hz,
                     config.diffusionDelays[1] * sampleRate_hz / 1000.0F);
    _allPass.setGain(config.diffusionDecays[1]);
    std::fill(_outs.begin(), _outs.end(), 0.0F);
  }

  void process(float sample) {

    // Lfo
    auto lfoValue = _lfo();
    // @todo : clamp lfo value delay?;

    // Modulated all pass. @todo: extend interface to do this two operations in
    // one step.
    _modAllPass.setDelaySamples(_config.diffusionDelays[0] +
                                lfoValue * _config.lfoMaxExcursion);
    sample = _modAllPass.process(sample);

    // delay 1
    auto delayed = _delays[0](_config.delays[0]);
    _delays[0].push(sample);
    _outs[0] = _delays[0](_config.outputTaps[0]);
    _outs[1] = _delays[0](_config.outputTaps[1]);
    _outs[4] = _delays[0](_config.outputTaps[4]);

    // Low pass filter
    sample = _dampingFilter.process(delayed);

    // all pass diffuser
    sample = _allPass.process(sample);
    // @todo : should the sample output be retrieved before or after.
    _outs[2] = _allPass.getSampleAtDelay(_config.outputTaps[2]);
    _outs[5] = _allPass.getSampleAtDelay(_config.outputTaps[5]);

    // delay 2
    delayed = _delays[1](_config.delays[1]);
    _delays[1].push(sample);
    _outs[3] = _delays[1](_config.outputTaps[3]);
    _outs[6] = _delays[1](_config.outputTaps[6]);

    _state = delayed;
  }

  Configuration _config;
  dsp::Lfo _lfo = {44100.0F, 1.0F};
  constexpr static float MAX_DELAY_MS = 100.0;
  constexpr static auto MAX_DELAY_IN_SAMPLES = (utilities::power_ceil(static_cast<size_t>(44100.0 * MAX_DELAY_MS / 1000.0)));

  dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES> _modAllPass = {44100.0F, 100.0F, 0.5F};
  dsp::AllPassFilter<MAX_DELAY_IN_SAMPLES> _allPass = {44100.0F, 100.0F, 0.5F};
  std::array<dsp::FractionalStaticDelay<MAX_DELAY_IN_SAMPLES>, 2U> _delays;
  dsp::OnePoleLpf _dampingFilter;
  float _damping = 0.5F;
  std::array<float, 7U> _outs;
  float _state = 0.0F;
};

class DattoroReverb {
public:
  void prepare(float sampleRate_hz) {

    TankBranch::Configuration tank0Config = {
        .lfoMaxExcursion{12},
        .delays{6241.0F, 4641.0F},
        .diffusionDecays{0.70F, 0.50F},
        .diffusionDelays{1343.0F, 3931.0F},
        .outputTaps{394.0F, 4401.0F, 2831.0F, 2954.0F, 3124.0F, 496.0F,
                    179.0F}};

    TankBranch::Configuration tank1Config = {
        .lfoMaxExcursion{12},
        .delays{6590.0F, 5505.0F},
        .diffusionDecays{0.70F, 0.50F},
        .diffusionDelays{995.0F, 2664.0F},
        .outputTaps{522.0F, 5368.0F, 1817.0F, 3956.0F, 2945.0F, 277.0F,
                    1578.0F}};

    _bandwithFilter.setFeedbackGain(_bandWidth);
    _diffuser.prepare(sampleRate_hz);
    _tanks[0].prepare(sampleRate_hz, tank0Config);
    _tanks[1].prepare(sampleRate_hz, tank1Config);
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

      auto delayed = _preDelay(_predelaySamples * _predelayPct);
      _preDelay.push(sample);
      // bandwith filter step
      sample = _bandwithFilter.process(delayed);
      // input diffuser

      auto out = sample;

      if (_diffuserEnabled) {
        sample = _diffuser.process(out);
      }

      out = sample;

      if (_tankEnabled) {
        auto tank0input = sample + _tanks[1]._state * _decay;
        auto tank1input = sample + _tanks[0]._state * _decay;
        // difussion step
        _tanks[0].process(tank0input);
        _tanks[1].process(tank1input);

        auto outs = conformOutput(_tanks[0]._outs, _tanks[1]._outs);

        out = (outs.first + outs.second) * 0.5;
      }

      if (out > 1.0F)
        out = 1.0;
      if (out < -1.0F)
        out = -1.0F;

      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        output[sample_i * numberOfChannels + channel_i] = out;
      }
    }
  }


  void process(float *const *signal, size_t numberOfChannels,
               size_t numberOfSamples) {

    for (size_t sample_i = 0; sample_i < numberOfSamples; sample_i++) {

      // Stereo to mono conversion.
      float sample = 0.0;
      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        sample += signal[channel_i][sample_i];
      }

      sample /= numberOfChannels;

      auto delayed = _preDelay(_predelaySamples);
      _preDelay.push(sample);
      // bandwith filter step
      sample = _bandwithFilter.process(delayed);
      // input diffuser

      auto out = sample;

      if (_diffuserEnabled) {
        sample = _diffuser.process(out);
      }

      out = sample;

      if (_tankEnabled) {
        auto tank0input = sample + _tanks[1]._state * _decay;
        auto tank1input = sample + _tanks[0]._state * _decay;
        // difussion step
        _tanks[0].process(tank0input);
        _tanks[1].process(tank1input);

        auto outs = conformOutput(_tanks[0]._outs, _tanks[1]._outs);

        out = (outs.first + outs.second) * 0.5;
      }

      if (out > 1.0F)
        out = 1.0;
      if (out < -1.0F)
        out = -1.0F;
      for (size_t channel_i = 0; channel_i < numberOfChannels; ++channel_i) {
        signal[channel_i][sample_i] = out;
      }
    }
  }


  void setDecay(float decay) { _decay = decay; }

  void setDamping(float damping) {
    for (auto &tank : _tanks) {
      tank._damping = damping;
    }
  }

  void setBandwith(float bandwidth) {
    _bandWidth = bandwidth;
    _bandwithFilter.setFeedbackGain(bandwidth);
  }

  void setPredelay(float preDelayPct){
    _predelayPct = preDelayPct;
  }

private:
  std::pair<float, float> conformOutput(const std::array<float, 7U> &tank0Out,
                                        const std::array<float, 7U> &tank1Out) {

    float yL = tank0Out[0] + tank0Out[1] - tank0Out[2] + tank0Out[3] -
               tank1Out[4] - tank1Out[5] - tank1Out[6];
    float yR = tank1Out[0] + tank1Out[1] - tank1Out[2] + tank1Out[3] -
               tank0Out[4] - tank0Out[5] - tank0Out[6];

    return {yL * 0.6F, yR * 0.6F};
  }

  InputDiffuser _diffuser;
  std::array<TankBranch, 2U> _tanks;
  constexpr static float _predelaySamples = 1000.0;
  float _predelayPct = 0.5;
  dsp::FractionalStaticDelay<1024> _preDelay;
  dsp::OnePoleLpf _bandwithFilter;
  float _decay = 0.5F;
  float _bandWidth = 0.5F;
  bool _tankEnabled = true;
  bool _diffuserEnabled = true;
};
