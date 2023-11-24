#pragma once

#include <cstdint>
#include "WavetableOsc.h"
#include "daisysp.h"
#include "daisy_seed.h"

namespace mirroraudio
{

class ThxVoice
{
public:
    void init(float sampleRate, float highestFreq);
    void update(float pitchShift, float freq);
    float process();
    static constexpr float LOWEST_FREQ = 36.0;
    static constexpr float HIGHEST_FREQ = 1500.0;
private:
    audioUtilities::WavetableOsc<64U> _osc;
    audioUtilities::WavetableOsc<64U> _lfo;
    float _oscFreq;
    float _targetFreq;
};


}
