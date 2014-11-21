#pragma once

namespace sma
{
template <typename Formatter>
class ObjectDataIn final
{
public:
  ObjectDataIn(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  template <typename T>
  T get();
  std::size_t get(std::uint8_t* dst, std::size_t size);

private:
  Formatter f;
};

template <typename Formatter>
template <typename T>
T ObjectDataIn<Formatter>::get()
{
  return f.get<T>();
}

template <typename Formatter>
std::size_t ObjectDataIn<Formatter>::get(std::uint8_t* dst, std::size_t size)
{
  return f.get(dst, size);
}
}
