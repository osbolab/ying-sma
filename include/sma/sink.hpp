#pragma once

namespace sma
{

template <typename T>
class csink
{
public:
  virtual ~csink() {}

  virtual void accept(const T& t) = 0;
};

template <typename T>
class sink : public csink<T>
{
public:
  virtual ~sink() {}

  virtual void accept(T t) = 0;
};

}
