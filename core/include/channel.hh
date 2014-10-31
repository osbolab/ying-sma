#pragma once

#include "sink.hh"
#include "blockingsource.hh"

#include <functional>


namespace sma
{

template<typename T>
class Channel : public Sink<T>
{
public:
  void onReceive(std::function<void(T&& t)> callback)
  {
    this->callback = std::move(callback);
  }

protected:
  bool onReceive(T&& t) const
  {
    auto copy = callback;
    if (!copy) return false;
    copy(std::move(t));
    return true;
  }

private:
  std::function<void(T&& t)> callback;
};

}