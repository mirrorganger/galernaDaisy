#include "Galerna.hpp"


namespace daisyGalerna{

constexpr std::size_t numberOfbits(std::size_t x)
{
    return x < 2 ? x : 1+numberOfbits(x >> 1);
}

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
    configureDigitalInputs();
    configureLeds();
}

void Galerna::processAnalogControls(){
    for (auto& pot : _pots){
        pot.Process();
    }
}

float Galerna::getPotValue(Pot pot){
    return _pots[static_cast<std::size_t>(pot)].Value();
}

bool Galerna::getSwitchState(Switch sw){
    return _switches[static_cast<std::size_t>(sw)].RawState();
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
    constexpr daisy::Pin PIN_ADC_MUX = daisy::seed::A4;
    constexpr std::array<daisy::Pin, numberOfbits(Galerna::NUM_POTS)-1U> POT_MUX_SEL_PINS = {daisy::seed::D20,daisy::seed::D21,daisy::seed::D22}; 
    constexpr std::array<uint8_t, NUM_POTS> POTS_PCB_ORDER = {3U,0U,1U,2U,4U,6U,7U,5U}; 
    daisy::AdcChannelConfig adcConfig;
    adcConfig.InitMux(PIN_ADC_MUX, Galerna::NUM_POTS,POT_MUX_SEL_PINS[0],POT_MUX_SEL_PINS[1],POT_MUX_SEL_PINS[2]);    
    hw.adc.Init(&adcConfig,1U);
    for (size_t pot_i = 0; pot_i < _pots.size(); pot_i++)
    {
        _pots[pot_i].Init(hw.adc.GetMuxPtr(0,POTS_PCB_ORDER[pot_i]),hw.AudioCallbackRate());
    }
}

void Galerna::configureDigitalInputs(){
    constexpr std::array<daisy::Pin, NUM_SWITCHES> SW_PINS = {daisy::seed::D16,daisy::seed::D15}; 
    for (size_t i = 0; i < NUM_SWITCHES; i++)
    {
        _switches[i].Init(dsy_gpio_pin(SW_PINS[i]));
    }
}

void Galerna::configureLeds(){
    std::array<daisy::Pin, Galerna::NUM_LEDS> LED_PINS = {daisy::seed::D23,daisy::seed::D24,daisy::seed::D25,daisy::seed::D26};
    for (size_t i = 0; i < Galerna::NUM_LEDS; i++)
    {
        _leds[i].Init(LED_PINS[i], false, 100);
    }  
}


}