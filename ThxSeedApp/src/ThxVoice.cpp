#include "ThxVoice.h"

using namespace daisysp;

namespace mirroraudio
{

void ThxVoice::init(float sampleRate, float highestFreq){
     _osc.setUp(audioUtilities::OscillatorType::SAWTOOTH,sampleRate);
     _lfo.setUp(audioUtilities::OscillatorType::SAWTOOTH,sampleRate);
    _targetFreq = highestFreq;
    _lfo.setFrequency(daisy::Random::GetFloat(50, 300) / 10.0);
}

void ThxVoice::update(float pitchShift, float pitch){
    auto oscFreqOffset = fmap(pitchShift, 0, 500);
    auto oscFreqMin = LOWEST_FREQ + oscFreqOffset;
    auto oscFreqMax = _targetFreq + oscFreqOffset;
    _oscFreq = fmap(pitch, oscFreqMin, oscFreqMax);
    auto lfoAmp = fmap(pitch,0.f, 0.005);
    _lfo.setAmplitude(lfoAmp);
}

float ThxVoice::process(){
    _osc.setFrequency(_oscFreq * (1.F + _lfo.process()));
    return _osc.process();
}

}


