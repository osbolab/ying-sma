#pragma once

#include <cstdint>
#include <utility>


namespace sma
{
template <typename Formatter>
class DataWriter
{
  using Myt = DataWriter<Formatter>;

public:
  DataWriter(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  template <typename T>
  Myt& put(T t)
  {
    f.put(std::forward<T>(t));
    return *this;
  }
  Myt& put(char const* src, std::size_t size)
  {
    f.put(src, size);
    return *this;
  }
  Myt& put(std::uint8_t const* src, std::size_t size)
  {
    f.put(src, size);
    return *this;
  }

protected:
  Formatter f;
};

template <typename Formatter, typename... Args>
DataWriter<Formatter> make_data_writer(Args&&... args)
{
  return DataWriter<Formatter>(Formatter(std::forward<Args>(args)...));
}
}
