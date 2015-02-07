#pragma once

#include <cstdlib>
#include <vector>
#include <cassert>


namespace sma
{
template <typename T, std::size_t Capacity>
class CircularBuffer
{
  static_assert(Capacity > 0, "Circular array must have nonzero capacity");

public:
  CircularBuffer()
    : sz(0)
    , first(0)
    , v(Capacity)
  {
  }

  CircularBuffer(CircularBuffer&&) = default;
  CircularBuffer& operator=(CircularBuffer&&) = default;

  std::size_t push_back(T value)
  {
    auto next = (first + sz) % Capacity;
    if (next == first)
      first = (first + 1) % Capacity;
    v[next] = std::move(value);
    if (sz < Capacity)
      ++sz;
    return next;
  }

  std::size_t size() const { return sz; }

  T& operator[](std::size_t const index)
  {
    assert(index < sz);
    return v[(first + index) % Capacity];
  }

  T const& operator[](std::size_t const index) const
  {
    assert(index < sz);
    return v[(first + index) % Capacity];
  }

  T& last() { return (*this)[sz - 1]; }
  T const& clast() const { return (*this)[sz - 1]; }

private:
  std::vector<T> v;

  std::size_t sz;
  std::size_t first;
};
}
