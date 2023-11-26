#include "daisy_seed.h"
#include "Galerna.hpp"
#include "daisysp.h"
#include <array>
#include <bitset>
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

static daisysp::ReverbSc reverb;
static galernaDaisy::Galerna galerna;
static daisy::Parameter reverbLpParam;
static daisy::Parameter dryWeight;
static daisy::Parameter feedback;

void UpdateKnobs(){
    reverb.SetLpFreq(reverbLpParam.Process());
    dryWeight.Process();
    reverb.SetFeedback(feedback.Process());
}

void Controls(){
    galerna.processAll();
    UpdateKnobs();
}

void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
    Controls();
    auto dry = galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0)? dryWeight.Value():1.0;

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

    galerna.setLed(galernaDaisy::Galerna::Led::LED_0,galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0));
    galerna.setLed(galernaDaisy::Galerna::Led::LED_1,galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_1));
    galerna.setLed(galernaDaisy::Galerna::Led::LED_2,dryWeight.Value());
    galerna.setLed(galernaDaisy::Galerna::Led::LED_3,feedback.Value());
}

int main(void)
{
    galerna.init();

    galerna.hw.SetAudioBlockSize(48);
    galerna.bindParameterToAnalogControl(reverbLpParam, galernaDaisy::Galerna::Pot::POT_0, 400,22000,daisy::Parameter::Curve::LOGARITHMIC);
    galerna.bindParameterToAnalogControl(dryWeight, galernaDaisy::Galerna::Pot::POT_1, 0.2,0.8,daisy::Parameter::Curve::LOGARITHMIC);
    galerna.bindParameterToAnalogControl(feedback, galernaDaisy::Galerna::Pot::POT_2, 0.2,0.8,daisy::Parameter::Curve::LOGARITHMIC);

    reverb.Init(galerna.hw.AudioSampleRate());
    reverb.SetLpFreq(18000.0f);
    reverb.SetFeedback(0.85f);

    galerna.hw.adc.Start();
	galerna.hw.StartAudio(AudioCallback);
	while(true) {
        System::Delay(30);
        UpdateLeds();
        galerna.displayControls(false);
    }
}
