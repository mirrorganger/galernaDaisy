#include "Galerna.hpp"
#include "ThxVoice.h"
#include "daisy_seed.h"
#include "daisysp.h"

#include <array>
#include <bitset>
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

static constexpr std::size_t NR_VOICES = 15;
std::array<mirroraudio::ThxVoice, NR_VOICES> DSY_SDRAM_BSS thxVoices;
daisysp::MoogLadder DSY_SDRAM_BSS flt;

static galernaDaisy::Galerna galerna;
static daisy::Parameter pitchParam;
static daisy::Parameter pitchShiftParam;
static daisy::Parameter timbreParam;
static daisy::Parameter reverbFeedbackParam;
static daisy::Parameter reverbLpFreqParam;
static daisy::Parameter numberOfVoicesParam;

bool trigger = false;

daisysp::ReverbSc DSY_SDRAM_BSS reverb;
Overdrive drive;

void UpdateInputs() {
  pitchParam.Process();
  pitchShiftParam.Process();
  timbreParam.Process();
  reverbFeedbackParam.Process();
  numberOfVoicesParam.Process();
  reverbLpFreqParam.Process();

  for (auto &voice : thxVoices) {
    voice.update(pitchParam.Value(), pitchShiftParam.Value());
  }
  reverb.SetFeedback(reverbFeedbackParam.Value());
  reverb.SetLpFreq(reverbLpFreqParam.Value());
  auto fltRes = fmap(timbreParam.Value(), 0, 0.5);
  flt.SetFreq(timbreParam.Value());
  flt.SetRes(fltRes);
}

void Controls() {
  galerna.processAll();
  UpdateInputs();
}

void AudioCallback(AudioHandle::InputBuffer, AudioHandle::OutputBuffer out,
                   size_t size) {
  Controls();
  auto isTriggered =
      galerna.getSwitchState(galernaDaisy::Galerna::Switch::SW_0);
  auto voicesToProcess = static_cast<size_t>(numberOfVoicesParam.Value() *
                                             static_cast<float>(NR_VOICES));
  for (size_t i = 0; i < size; i++) {
    float output = .0;
    for (size_t voice = 0; voice < voicesToProcess; voice++) {
      output += thxVoices[voice].process();
    }
    // for(auto& voice : thxVoices)
    output = flt.Process(output) * 3.0F / voicesToProcess;
    float sig = output;
    if (isTriggered) {
      sig = drive.Process(output);
    }
    reverb.Process(sig, sig, &out[0][i], &out[1][i]);
  }
}

void UpdateLeds() {

  galerna.setLed(galernaDaisy::Galerna::Led::LED_0,
                 galerna.getButtonState(galernaDaisy::Galerna::Button::BTN_0));
  galerna.setLed(galernaDaisy::Galerna::Led::LED_1,
                 galerna.getButtonState(galernaDaisy::Galerna::Button::BTN_1));
  galerna.setLed(galernaDaisy::Galerna::Led::LED_2, pitchParam.Value());
  galerna.setLed(galernaDaisy::Galerna::Led::LED_3,
                 numberOfVoicesParam.Value());
}

int main() {

  galerna.init();

  galerna.hw.SetAudioBlockSize(16);
  galerna.bindParameterToAnalogControl(
      pitchParam, galernaDaisy::Galerna::Pot::POT_0, 0.2F, 0.8F,
      daisy::Parameter::Curve::LOGARITHMIC);
  galerna.bindParameterToAnalogControl(
      pitchShiftParam, galernaDaisy::Galerna::Pot::POT_1, 0.2F, 0.8F,
      daisy::Parameter::Curve::LOGARITHMIC);
  galerna.bindParameterToAnalogControl(
      timbreParam, galernaDaisy::Galerna::Pot::POT_2, 60, 1500,
      daisy::Parameter::Curve::LOGARITHMIC);
  galerna.bindParameterToAnalogControl(
      reverbFeedbackParam, galernaDaisy::Galerna::Pot::POT_3, 0.01F, 0.8F,
      daisy::Parameter::Curve::LOGARITHMIC);
  galerna.bindParameterToAnalogControl(
      reverbLpFreqParam, galernaDaisy::Galerna::Pot::POT_4, 0.2F, galerna.hw.AudioSampleRate() / 2.5,
      daisy::Parameter::Curve::LOGARITHMIC);
  galerna.bindParameterToAnalogControl(numberOfVoicesParam,
                                       galernaDaisy::Galerna::Pot::POT_5, 0.2F,
                                       1.0F, daisy::Parameter::Curve::LINEAR);

  reverb.Init(galerna.hw.AudioSampleRate());

  auto lowestFreq = mirroraudio::ThxVoice::LOWEST_FREQ;

  reverb.Init(galerna.hw.AudioSampleRate());
  reverb.SetFeedback(0.9f);
  reverb.SetLpFreq(galerna.hw.AudioSampleRate() / 2.5);
  for (auto &voice : thxVoices) {
    voice.init(galerna.hw.AudioSampleRate(), lowestFreq);
    lowestFreq *= 1.5F;
    if (lowestFreq > mirroraudio::ThxVoice::HIGHEST_FREQ)
      lowestFreq *= 0.5F;
  }
  drive.Init();
  flt.Init(galerna.hw.AudioSampleRate());
  galerna.hw.adc.Start();
  galerna.hw.StartAudio(AudioCallback);

  galerna.displayInitialText("Galerna : ThxSeed");

  while (true) {
    System::Delay(10);
    UpdateLeds();
    galerna.displayControls(false);
  }
}
