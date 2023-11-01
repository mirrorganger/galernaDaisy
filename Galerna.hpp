#pragma once

#include "daisy_seed.h"
#include <cstdint>
#include <array>

namespace daisyGalerna
{


class Galerna
{
public:

    enum class Pot : uint_fast8_t 
    {
        POT_0 = 0U,
        POT_1 = 1U,
        POT_2 = 2U,
        POT_3 = 3U,
        POT_4 = 4U,
        POT_5 = 5U,
        POT_6 = 6U,
        POT_7 = 7U
    };

    enum class Led : uint_fast8_t 
    {
        LED_0 = 0U,
        LED_1 = 1U,
        LED_2 = 2U,
        LED_3 = 3U,
        LED_LAST = 4U
    };

    void init();
    void processAnalogControls();
    float GetPotValue(Pot pot);
    void bindParameterToAnalogControl(daisy::Parameter& param, Pot pot, float min, float max, daisy::Parameter::Curve curve);
    void setLed(Led led, float brigthness);
    void updateLeds();


    daisy::DaisySeed hw;

private:
    void configureAnanalogControls();
    void configureLeds();

    static constexpr std::size_t NUM_POTS = 8U;
    static constexpr std::size_t NUM_LEDS = 4U;

    std::array<daisy::AnalogControl,NUM_POTS> _pots;
    std::array<daisy::Led,NUM_LEDS> _leds;

};
}

