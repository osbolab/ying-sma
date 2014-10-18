#pragma once

namespace sma
{

template<class T>
class Sink
{
public:
  virtual bool sink(T item) = 0;
};

}