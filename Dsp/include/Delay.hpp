#pragma once

#include "CircularBuffer.h"

namespace dsp {

template <typename BaseType> class DelayT : public BaseType {
public:
  using index_type = typename BaseType::index_type;

  explicit DelayT(float delay_ms, float sampleRate)
      : BaseType(std::size_t(delay_ms * sampleRate)) {}

  explicit DelayT(float delayInSamples)
      : BaseType(std::size_t(delayInSamples)) {}

  explicit DelayT() = default;

  // returns maximum delay
  float operator()() const { return this->back(); }

  // returns delay at index
  float operator()(index_type index) const { return (*this)[index]; }
};

using FractionalDelay = DelayT<utilities::CircularBufferFrac<float>>;

using Delay = DelayT<utilities::CircularBuffer<float>>;

template<size_t N>
using FractionalStaticDelay = DelayT<utilities::CircularBufferFrac<float,std::array<float,N>>>;

template<size_t N>
using StaticDelay = DelayT<utilities::CircularBuffer<float,std::array<float,N>>>;


} // namespace dsp