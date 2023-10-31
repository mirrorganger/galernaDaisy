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
        POT_7 = 7U,
        POT_LAST = 8U
    };

    enum class Leds : uint_fast8_t 
    {
        LED_0 = 0U,
        LED_1 = 1U,
        LED_2 = 2U,
        LED_3 = 3U,
        LED_LAST = 4U
    };

    float GetPotValue(Pot pot);

private:
    void configureAnanalogControls();


    std::array<daisy::AnalogControl,static_cast<std::size_t>(Pot::POT_LAST)> _pots;
    std::array<daisy::Led,static_cast<std::size_t>(Pot::POT_LAST)> _leds;

};
}

