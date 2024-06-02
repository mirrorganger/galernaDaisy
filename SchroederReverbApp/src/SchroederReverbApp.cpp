#include "daisy_seed.h"
#include "Galerna.hpp"
#include "daisysp.h"
#include <array>
#include <bitset>
#include "SchroederReverb.h"
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

static SchroederReverb reverb;
galernaDaisy::Galerna  galerna;
static daisy::Parameter reverbLpParam;
static daisy::Parameter dryWeight;
static daisy::Parameter feedback;

void UpdateKnobs(){
    reverb.setDamping(reverbLpParam.Process());
    reverb.updateReverbTime(feedback.Process());
    dryWeight.Process();
    auto dry = galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0)? dryWeight.Value():0.0;
    reverb.setDryWetMix(dry);
}

void Controls(){
    galerna.processAll();
    UpdateKnobs();
}


void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{

    Controls();
    if(galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_1)){
        reverb.process(in,out,2U,size / 2U);
    }   
}

void UpdateLeds(){

    galerna.setLed(galernaDaisy::Galerna::Led::LED_0,galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0));
    galerna.setLed(galernaDaisy::Galerna::Led::LED_1,galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_1));
    galerna.setLed(galernaDaisy::Galerna::Led::LED_2,dryWeight.Value());
    galerna.setLed(galernaDaisy::Galerna::Led::LED_3,reverbLpParam.Value());
}

int main(void)
{
    galerna.init();

    galerna.hw.SetAudioBlockSize(48);
    galerna.bindParameterToAnalogControl(reverbLpParam, galernaDaisy::Galerna::Pot::POT_0, 0.1,0.99,daisy::Parameter::Curve::LOGARITHMIC);
    galerna.bindParameterToAnalogControl(dryWeight, galernaDaisy::Galerna::Pot::POT_1, 0.0,1.0,daisy::Parameter::Curve::LOGARITHMIC);
    galerna.bindParameterToAnalogControl(feedback, galernaDaisy::Galerna::Pot::POT_2, 100,2000,daisy::Parameter::Curve::LOGARITHMIC);

    galerna.hw.adc.Start();
	galerna.hw.StartAudio(AudioCallback);
    galerna.displayInitialText("SchroederReverb");

    reverb.prepare(galerna.hw.AudioSampleRate(),80);
    reverb.setDamping(.1);
    reverb.setDryWetMix(.5);
	while(true) {
        System::Delay(30);
        UpdateLeds();
        galerna.displayControls(false);
    }
}
