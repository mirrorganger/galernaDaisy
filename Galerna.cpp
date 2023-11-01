#include "Galerna.hpp"


namespace daisyGalerna{

template<size_t N>
inline std::array<daisy::AdcChannelConfig, N> initAdcPins(const std::array<daisy::Pin, N>& pins){
	std::array<daisy::AdcChannelConfig, N> channelConfigs;
	for (size_t i = 0; i < pins.size(); i++)
	{
		channelConfigs[i].InitSingle(pins[i]);
	}
	return channelConfigs;
}

void Galerna::init(){
    hw.Configure();
    hw.Init();
    configureAnanalogControls();
}

void Galerna::processAnalogControls(){
    for (auto& pot : _pots){
        pot.Process();
    }
}

float Galerna::GetPotValue(Pot pot){
    return _pots[static_cast<std::size_t>(pot)].Value();
}

void Galerna::bindParameterToAnalogControl(daisy::Parameter& param, Pot pot, float min, float max, daisy::Parameter::Curve curve){
    param.Init(_pots[static_cast<std::size_t>(pot)],min,max,curve);
}


void Galerna::configureAnanalogControls(){

    using namespace daisy::seed;
    std::array<daisy::Pin, Galerna::NUM_POTS> POT_PINS = {A0,A1,A2,A3,A4,A5,A6,A7};

    auto adcConfigs = initAdcPins(POT_PINS);

    hw.adc.Init(adcConfigs.data(),adcConfigs.size());
    for (size_t pot_i = 0; pot_i < _pots.size(); pot_i++)
    {
        _pots[pot_i].Init(hw.adc.GetPtr(pot_i),hw.AudioCallbackRate());
    }
}



}