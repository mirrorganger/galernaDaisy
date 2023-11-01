#include "daisy_seed.h"
#include "Galerna.hpp"
#include "daisysp.h"
#include <array>
#include <bitset>
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

static daisysp::ReverbSc reverb;
static daisyGalerna::Galerna galerna;
static daisy::Parameter reverbLpParam;
static daisy::Parameter dryWeight;

void UpdateKnobs(){
    reverb.SetLpFreq(reverbLpParam.Process());
    dryWeight.Process();
    reverb.SetFeedback(galerna.GetPotValue(daisyGalerna::Galerna::Pot::POT_3));
}

void Controls(){
    galerna.processAnalogControls();
    UpdateKnobs();
}

void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
    Controls();
    auto dry = dryWeight.Value();

    for (size_t i = 0; i < size; i+=2)
    {
        float sigl = in[i];
        float sigr = in[i+1];

        float verbl;
        float verbr;
        reverb.Process(sigl, sigr, &verbl, &verbr);
        
        out[i] = (sigl * dry) + (1.0f-dry) * verbl;
        out[i+1] = (sigr * dry) + (1.0f-dry) * verbr;
    }
    

}

void UpdateLeds(){
    galerna.setLed(daisyGalerna::Galerna::Led::LED_0,galerna.GetPotValue(daisyGalerna::Galerna::Pot::POT_3));
    galerna.setLed(daisyGalerna::Galerna::Led::LED_1,dryWeight.Value());
    galerna.updateLeds();
}

int main(void)
{
    galerna.init();

    galerna.hw.SetAudioBlockSize(16);
    galerna.bindParameterToAnalogControl(reverbLpParam, daisyGalerna::Galerna::Pot::POT_0, 400,22000,daisy::Parameter::Curve::LOGARITHMIC);
    galerna.bindParameterToAnalogControl(dryWeight, daisyGalerna::Galerna::Pot::POT_1, 0.2,0.8,daisy::Parameter::Curve::LOGARITHMIC);
    reverb.Init(galerna.hw.AudioSampleRate());

    //reverb parameters
    reverb.SetLpFreq(18000.0f);
    reverb.SetFeedback(0.85f);

    galerna.hw.StartLog(false);
    galerna.hw.adc.Start();
	galerna.hw.StartAudio(AudioCallback);
	while(1) {
        System::Delay(10);
        UpdateLeds();
    }
}
