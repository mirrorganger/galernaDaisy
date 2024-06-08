#include "DattoroReverb.h"
#include "Galerna.hpp"
#include "daisy_seed.h"
#include "daisysp.h"
#include "BiquadFilter.h"

#include <array>
#include <bitset>
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

static DattoroReverb reverb;
static dsp::BiquadFilter lowPassFilter;
Overdrive drive;

galernaDaisy::Galerna galerna;
static daisy::Parameter reverbLpParam;
static daisy::Parameter bandiwthParam;
static daisy::Parameter feedbackParam;
static daisy::Parameter preDelayParam;
static daisy::Parameter driveParam;
static daisy::Parameter lowPassFilterFreq;


bool isTriggered = false;
bool emanñe = false;

void UpdateKnobs() {
  reverb.setDamping(reverbLpParam.Process());
  reverb.setDecay(feedbackParam.Process());
  reverb.setBandwith(bandiwthParam.Process());
  reverb.setPredelay(preDelayParam.Process());
  drive.SetDrive(driveParam.Process());
  lowPassFilter.setCentralFreq(lowPassFilterFreq.Process());
  isTriggered = galerna.getButtonState(galernaDaisy::Galerna::Button::BTN_0);
}

void Controls() {
  galerna.processAll();
  UpdateKnobs();
}

void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out, size_t size) {

  Controls();
  if (galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_1)) {
    reverb.process(in, out, 2U, size / 2U);
    if (isTriggered) {
      for (size_t i = 0; i < size; i += 2) {
        auto aux = drive.Process(out[i]);
        out[i] = aux;
        out[i + 1] = aux;
      }
    }
    if(galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0)){
      lowPassFilter.process(out,out,2U, size / 2U);
    }



  } else {
    for (size_t i = 0; i < size; i += 2) {
      out[i] = in[i];
      out[i + 1] = in[i + 1];
    }
  }
}

void UpdateLeds() {

  galerna.setLed(galernaDaisy::Galerna::Led::LED_0,
                 galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0));
  galerna.setLed(galernaDaisy::Galerna::Led::LED_1,
                 galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_1));
  galerna.setLed(galernaDaisy::Galerna::Led::LED_2, bandiwthParam.Value());
  galerna.setLed(galernaDaisy::Galerna::Led::LED_3, reverbLpParam.Value());
}

int main(void) {
  galerna.init();

  galerna.hw.SetAudioBlockSize(48);
  galerna.bindParameterToAnalogControl(
      reverbLpParam, galernaDaisy::Galerna::Pot::POT_0, 0.01, 0.99,
      daisy::Parameter::Curve::LINEAR);
  galerna.bindParameterToAnalogControl(
      bandiwthParam, galernaDaisy::Galerna::Pot::POT_1, 0.01, 0.99,
      daisy::Parameter::Curve::LINEAR);
  galerna.bindParameterToAnalogControl(
      feedbackParam, galernaDaisy::Galerna::Pot::POT_2, 0.01, 0.99,
      daisy::Parameter::Curve::LINEAR);
  galerna.bindParameterToAnalogControl(
      preDelayParam, galernaDaisy::Galerna::Pot::POT_3, 0.01, 0.99,
      daisy::Parameter::Curve::LINEAR);
  galerna.bindParameterToAnalogControl(
      driveParam, galernaDaisy::Galerna::Pot::POT_4, 0.01, 0.99,
      daisy::Parameter::Curve::LINEAR);
  galerna.bindParameterToAnalogControl(
      lowPassFilterFreq, galernaDaisy::Galerna::Pot::POT_5, 200, 2000,
      daisy::Parameter::Curve::LOGARITHMIC);

  galerna.hw.adc.Start();
  galerna.hw.StartAudio(AudioCallback);
  galerna.displayInitialText("Dattoro Reverb");
  drive.Init();
  lfo.reset(galerna.hw.AudioSampleRate(),15);

  lowShelfFilter.setUp(dsp::BiquadFilter::FilterSettings{
    galerna.hw.AudioSampleRate(),
    sqrt(2.0)/2.0,
    800,
    dsp::BiquadFilter::Type::LOWPASS,
    2U,
    12});

  reverb.prepare(galerna.hw.AudioSampleRate());
  while (true) {
    System::Delay(30);
    UpdateLeds();
    galerna.displayControls(false);
  }
}
