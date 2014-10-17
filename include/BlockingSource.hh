#pragma once

namespace sma
{

template<class T>
class BlockingSource
{
public:
  virtual bool poll(T& item) = 0;
  virtual T take() = 0;
};

}