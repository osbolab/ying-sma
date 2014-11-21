#pragma once

#include <utility>

namespace sma
{
template <typename Formatter>
class ObjectDataOut final
{
public:
  ObjectDataOut(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  template <typename T>
  ObjectDataOut<Formatter>& put(T t);
  ObjectDataOut<Formatter>& put(std::uint8_t const* src, std::size_t size);

private:
  Formatter f;
};


template <typename Formatter>
template <typename T>
ObjectDataOut<Formatter>& ObjectDataOut<Formatter>::put(T t)
{
  f.put(std::forward<T>(t));
  return *this;
}

template <typename Formatter>
ObjectDataOut<Formatter>& ObjectDataOut<Formatter>::put(std::uint8_t const* src,
                                                        std::size_t size)
{
  f.put(src, size);
  return *this;
}
}
