#pragma once

namespace sma
{

template<class T>
class Sink
{
public:
  virtual void sink(T item) = 0;
};

}