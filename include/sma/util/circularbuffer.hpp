#pragma once

#include <cstdlib>
#include <vector>
#include <cassert>


namespace sma
{
template <typename T, std::size_t Capacity>
class CircularBuffer
{
  static_assert(Capacity > 0);

public:
  CircularBuffer(CircularBuffer&&) = default;
  CircularBuffer& operator=(CircularBuffer&&) = default;

  std::size_t push_back(T value)
  {
    auto next = (first + size) % Capacity;
    if (next == first)
      first = (first + 1) % Capacity;
    v[next] = std::move(value);
    return next;
  }

  std::size_t size() const { return size; }

  V& operator[](std::size_t const index)
  {
    assert(index < size);
    return v[(first + index) % Capacity];
  }

  V const& operator[](std::size_t const index) const { return (*this)[index]; }

private:
  std::vector<T> v;

  std::size_t size{0};
  std::size_t first{0};
};
}
