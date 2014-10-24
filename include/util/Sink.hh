#pragma once

namespace sma
{

template<typename T>
class Sink
{
public:
  virtual ~Sink() {}
  virtual bool sink(T item) = 0;
};

}