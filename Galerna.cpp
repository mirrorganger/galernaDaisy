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
    configureLeds();
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

void Galerna::setLed(Led led, float brightness){
    _leds[static_cast<std::size_t>(led)].Set(brightness);
}

void Galerna::updateLeds(){
    for (auto led : _leds)
    {
        led.Update();
    }    
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

void Galerna::configureLeds(){
    using namespace daisy::seed;
    std::array<daisy::Pin, Galerna::NUM_LEDS> LED_PINS = {D0,D1,D2,D3};
    for (size_t i = 0; i < Galerna::NUM_LEDS; i++)
    {
        _leds[i].Init(LED_PINS[i], false, 100);
    }
    
}


}