#include "Galerna.hpp"


namespace daisyGalerna{

static constexpr daisy::Pin PITCH_POT = daisy::seed::A0;
static constexpr daisy::Pin PITCH_SHIFT_POT = daisy::seed::A1;
static constexpr daisy::Pin TIMBRE_POT = daisy::seed::A2;
static constexpr daisy::Pin REV_POT = daisy::seed::A3;

static constexpr std::array<daisy::Pin, static_cast<std::size_t>(Galerna::Pot::POT_LAST)> POT_PINS = {
    daisy::seed::A0,
    daisy::seed::A1,
    daisy::seed::A2,
    daisy::seed::A3,
    daisy::seed::A4,
    daisy::seed::A5,
    daisy::seed::A6,
    daisy::seed::A7 
};


float Galerna::GetPotValue(Pot pot){

}

void Galerna::configureAnanalogControls(){

}



}