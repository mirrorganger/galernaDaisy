#ifndef AUDIOUTILITES_WAVETABLEOSC
#define AUDIOUTILITES_WAVETABLEOSC

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <algorithm>
#include <math.h>

namespace audioUtilities
{
using BuilderFunc = std::function<void(std::vector<float> &)>;

constexpr uint8_t HIGHER_HARMONIC = 48;


template<std::size_t SIZE>
float interpolation(const std::array<float, SIZE> buffer, float indexPtr)
{
    std::size_t idxBelow = floorf(indexPtr);
    std::size_t idxAbove = idxBelow + 1;
    if(idxAbove >= buffer.size())
        idxAbove = 0U;

    float weigthAbove = static_cast<float>(indexPtr - idxBelow);

    return (1.0 - weigthAbove) * buffer[idxBelow]
           + weigthAbove * buffer[idxAbove];
}


enum class OscillatorType : uint8_t
{
    SINE,
    SAWTOOTH,
    TRIANGLE,
    SQUARE
};


template <std::size_t TABLE_SIZE>
class WavetableOsc
{
  public:
    WavetableOsc() {}
    WavetableOsc(const OscillatorType oscillatorType,
                 const float          sampleRate,
                 const bool           useInterpolation = true)
    {
    }
    void setUp(const OscillatorType oscillatorType,
               const float          sampleRate,
               const bool           useInterpolation = true)
    {
        _inverseSampleRate = 1.0 / sampleRate;
        _useInterpolation  = useInterpolation;
        _readPointer       = 0;
        std::fill(_buffer.begin(), _buffer.end(), 0.0F);
        switch(oscillatorType)
        {
            case OscillatorType::SINE: makeSineOscillator(); break;
            case OscillatorType::TRIANGLE: makeTriangleOscillator(); break;
            case OscillatorType::SQUARE: makeSquareOscillator(); break;
            case OscillatorType::SAWTOOTH: makeSawToothOscillator(); break;
        }
    }
    // Sets oscillator frequency
    void setFrequency(const float f) { _freq = f; }
    // Sets the amplitude
    void setAmplitude(float amplitude) { _amplitude = amplitude; }
    // Gets next sample and update the phase
    float process()
    {
        float outValue = 0.0;

        if(_buffer.size() == 0)
            return outValue;

        _readPointer += _buffer.size() * _freq * _inverseSampleRate;
        while(_readPointer >= _buffer.size())
            _readPointer -= _buffer.size();

        if(_useInterpolation)
            outValue = interpolation(_buffer, _readPointer);
        else
            outValue = _buffer[(int)_readPointer];

        return outValue * _amplitude;
    }

  private:
    void makeTriangleOscillator()
    {
        std::generate(_buffer.begin(), _buffer.end(), [&, n = 0]() mutable {
            return sinf(2.0 * M_PI * static_cast<float>(n++)
                        / static_cast<float>(_buffer.size()));
        });
    }
    void makeSineOscillator()
    {
        std::generate(_buffer.begin(),
                      _buffer.begin() + _buffer.size() / 2,
                      [&, n = 0]() mutable {
                          return -1.0
                                 + 4.0 * static_cast<float>(n++)
                                       / static_cast<float>(_buffer.size());
                      });
        std::generate(
            _buffer.begin() + _buffer.size() / 2,
            _buffer.end(),
            [&, n = _buffer.size() / 2]() mutable {
                return 1.0
                       - 4.0 * static_cast<float>(n++ - _buffer.size() / 2)
                             / static_cast<float>(_buffer.size());
            });
    }

    void makeSquareOscillator()
    {
        std::fill(_buffer.begin(), _buffer.begin() + _buffer.size() / 2, 1.0);
        std::fill(_buffer.begin() + _buffer.size() / 2, _buffer.end(), -1.0);
    }

    void makeSawToothOscillator()
    {
        std::generate(_buffer.begin(), _buffer.end(), [&, n = 0]() mutable {
            float val = 0.0;
            for(uint8_t harmonic = 1; harmonic <= HIGHER_HARMONIC; harmonic++)
            {
                val += sinf(2.0 * M_PI * static_cast<float>(harmonic)
                            * static_cast<float>(n)
                            / static_cast<float>(_buffer.size()))
                       / static_cast<float>(harmonic);
            }
            n++;
            return val;
        });
    }

    std::array<float, TABLE_SIZE> _buffer;
    OscillatorType                _oscillatorType;
    float                         _inverseSampleRate;
    float                         _freq;
    float                         _readPointer;
    bool                          _useInterpolation;
    float                         _amplitude = .5;
};

} // namespace audioUtilities

#endif
