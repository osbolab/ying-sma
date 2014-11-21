#pragma once

#include <cstdint>


namespace sma
{
template <typename Formatter>
class ObjectDataIn final
{
  using Myt = ObjectDataIn<Formatter>;

public:
  ObjectDataIn(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  template <typename T>
  T get()
  {
    return f.template get<T>();
  }
  Myt& get(std::int8_t* dst, std::size_t size)
  {
    f.template get(dst, size);
    return *this;
  }
  Myt& get(std::uint8_t* dst, std::size_t size)
  {
    f.template get(dst, size);
    return *this;
  }

private:
  Formatter f;
};
}
