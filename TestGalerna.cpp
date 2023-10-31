#include "daisy_seed.h"
#include "Galerna.hpp"
#include "daisysp.h"
#include <array>
#include <bitset>
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;


DaisySeed hw;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
}


int main(void)
{

    hw.Init();
    hw.SetAudioBlockSize(16);

    hw.adc.Start();
	hw.StartAudio(AudioCallback);


	while(1) {
    }
}
