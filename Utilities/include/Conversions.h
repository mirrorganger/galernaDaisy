//
// Created by cesar on 11/14/22.
//

#ifndef EUTERPESND_CONVERSIONS_H
#define EUTERPESND_CONVERSIONS_H

#include <cmath>
#include <cstdint>

namespace utilities {

class Conversions {
public:
    static double pitchToFrequency(double pitch) {
    double exponent = (pitch - static_cast<double>(MIDLE_C_PITH)) *
                      static_cast<double>(1.0f / SEMITONES_PER_OCTAVE);
    return MIDLE_C_FREQUENCY * std::pow(2.0, exponent);
  }

private:
  static constexpr uint32_t SEMITONES_PER_OCTAVE = 12U;
  static constexpr uint32_t MIDLE_C_PITH = 60U;
  static constexpr float MIDLE_C_FREQUENCY = 261.625549F;
};

template <typename DataType>
inline DataType fromGainToDecibels(DataType gain,
                                   DataType minimumLevelDb = (DataType)-100) {
  return (gain > (DataType)0)
             ? std::max(std::log10(gain) * static_cast<DataType>(20),
                        minimumLevelDb)
             : minimumLevelDb;
}

template <typename DataType>
inline DataType fromDecibelsToGain(DataType decibels,
                                   DataType minimumLevelDb = (DataType)-100) {
  return (decibels > minimumLevelDb)
             ? std::pow(DataType(10.0), decibels / DataType(20.0))
             : DataType(0);
}

} // namespace utilities

#endif // EUTERPESND_CONVERSIONS_H
