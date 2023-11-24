#pragma once

#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"
#include <cstdint>
#include <array>

namespace galernaDaisy
{


class Galerna
{
public:

    using GalernaDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;

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
    };

    enum class Switch: uint_fast8_t{
        SW_0 = 0U,
        SW_1 = 1U
    };

    void init();
    void processAnalogControls();
    float getPotValue(Pot pot);
    bool getSwitchState(Switch sw);
    void bindParameterToAnalogControl(daisy::Parameter& param, Pot pot, float min, float max, daisy::Parameter::Curve curve);
    void setLed(Led led, float brigthness);
    void updateLeds();
    void displayControls(bool invert);

    daisy::DaisySeed hw;

private:
    void configureAnanalogControls();
    void configureDigitalInputs();
    void configureLeds();
    void configureDisplay();

    static constexpr std::size_t NUM_POTS = 8U;
    static constexpr std::size_t NUM_LEDS = 4U;
    static constexpr std::size_t NUM_SWITCHES = 2U;

    std::array<daisy::AnalogControl,NUM_POTS> _pots;
    std::array<daisy::Led,NUM_LEDS> _leds;
    std::array<daisy::Switch,NUM_SWITCHES> _switches;
    GalernaDisplay _display;
    uint32_t _screenUpdatePeriod;
    uint32_t _lastScreenUpdate;
};
}
