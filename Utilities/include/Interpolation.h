#pragma once

#include <cmath>

namespace utilities {

struct linear_interpolation {
  template <typename BufferType, typename DataType>
  DataType operator()(BufferType const &buffer, DataType index) const {
    auto prev = buffer[std::size_t(index)];
    auto next = buffer[std::size_t(index) + 1];
    auto frac = index - std::floor(index);
    return prev + frac * (next - prev);
  }
};

} // namespace utilities