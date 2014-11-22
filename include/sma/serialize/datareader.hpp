#pragma once

#include <cstdint>
#include <utility>


namespace sma
{
template <typename Formatter>
class DataReader
{
  using Myt = DataReader<Formatter>;

public:
  DataReader(Formatter formatter)
    : f(std::move(formatter))
  {
  }

  template <typename T>
  T get()
  {
    return f.template get<T>();
  }
  Myt& get(char* dst, std::size_t size)
  {
    f.template get(dst, size);
    return *this;
  }
  Myt& get(std::uint8_t* dst, std::size_t size)
  {
    f.template get(dst, size);
    return *this;
  }

protected:
  Formatter f;
};


template <typename Formatter, typename... Args>
DataReader<Formatter> make_data_reader(Args&&... args)
{
  return DataReader<Formatter>(Formatter(std::forward<Args>(args)...));
}
}
