#pragma once
#include <array>
#include <vector>

#include "Interpolation.h"

namespace utilities {

template <typename T> constexpr T power_ceil(T x) {
  if (x <= 1)
    return 1;
  int power = 2;
  x--;
  while (x >>= 1)
    power <<= 1;
  return power;
}

template <typename T> constexpr bool is_powerof2(T v) {
  return v && ((v & (v - 1)) == 0);
}

template <typename T>
void initialize_buffer(std::size_t size, std::vector<T> &buffer,
                       std::size_t &mask) {
  std::size_t required_size = power_ceil(size);
  mask = required_size - 1;
  buffer.resize(required_size, T{});
}

template <typename T, std::size_t N>
void initialize_buffer(std::array<T, N> &buffer, std::size_t &mask) {
  static_assert(is_powerof2(N), "Size must a power of two");
  mask = buffer.size() - 1;
}

template <typename T, typename BufferT = std::vector<T>> class CircularBuffer {
public:
  using index_type = std::size_t;

  explicit CircularBuffer();
  explicit CircularBuffer(std::size_t size);
  CircularBuffer(CircularBuffer const &rhs) = default;
  CircularBuffer(CircularBuffer &&rhs) = default;

  CircularBuffer &operator=(CircularBuffer const &rhs) = default;
  CircularBuffer &operator=(CircularBuffer &&rhs) = default;

  T const &operator[](std::size_t index) const;
  T &operator[](std::size_t index);
  std::size_t size() const;
  // Push a value into the buffer
  void push(T val);
  // Returns the latest element.
  T const &front() const;
  T &front();
  // Clears the buffer
  void clear();
  // Remove the latest element
  void pop_front();
  // Returns the oldest element.
  T const &back() const;
  T &back();

private:
  std::size_t _mask;
  std::size_t _writePos = 0;
  BufferT _buffer;
};

template <typename T, typename BufferT>
CircularBuffer<T, BufferT>::CircularBuffer() {
  initialize_buffer(_buffer, _mask);
}

template <typename T, typename BufferT>
CircularBuffer<T, BufferT>::CircularBuffer(std::size_t size) {
  initialize_buffer(size, _buffer, _mask);
}

template <typename T, typename BufferT>
T const &CircularBuffer<T, BufferT>::operator[](std::size_t index) const {
  return _buffer[(_writePos + index) & _mask];
}

template <typename T, typename BufferT>
T &CircularBuffer<T, BufferT>::operator[](std::size_t index) {
  return _buffer[(_writePos + index) & _mask];
}

template <typename T, typename BufferT>
std::size_t CircularBuffer<T, BufferT>::size() const {
  return _buffer.size();
}

template <typename T, typename BufferT>
void CircularBuffer<T, BufferT>::push(T newValue) {
  --_writePos &= _mask;
  _buffer[_writePos] = newValue;
}

template <typename T, typename BufferT>
T const &CircularBuffer<T, BufferT>::front() const {
  return (*this)[0];
}

template <typename T, typename BufferT> T &CircularBuffer<T, BufferT>::front() {
  return (*this)[0];
}

template <typename T, typename BufferT>
void CircularBuffer<T, BufferT>::clear() {
  for (auto &element : _buffer) {
    element = T{};
  }
}

template <typename T, typename BufferT>
void CircularBuffer<T, BufferT>::pop_front() {
  _writePos++;
}

template <typename T, typename BufferT>
T const &CircularBuffer<T, BufferT>::back() const {
  return (*this)[size() - 1];
}

template <typename T, typename BufferT> T &CircularBuffer<T, BufferT>::back() {
  return (*this)[size() - 1];
}

template <typename T, typename BufferT = std::vector<T>,
          typename IndexType = float>
class CircularBufferFrac : public CircularBuffer<T, BufferT> {
public:
  using base_type = CircularBuffer<T, BufferT>;
  using index_type = IndexType;

  using CircularBuffer<T, BufferT>::CircularBuffer;

  T operator[](IndexType index) const {
    linear_interpolation interpolation;
    return interpolation(static_cast<base_type const &>(*this), index);
  }
};

} // namespace utilities
