#pragma once

namespace sma
{

template <typename T>
class sink
{
public:
  virtual ~sink() {}

  virtual void accept(T t) = 0;
};
}
