#pragma once

#include <functional>

#include "Sink.hh"
#include "BlockingSource.hh"


namespace sma
{

template<class T>
class Channel : public Sink<T>
{
public:
  void onReceive(std::function<void(T&& t)> callback);

protected:
  bool onReceive(T&& t) const;

private:
  std::function<void(T&& t)> callback;
};


template<class T>
void Channel<T>::onReceive(std::function<void(T&& t)> callback)
{
  this->callback = std::move(callback);
}

template<class T>
bool Channel<T>::onReceive(T&& t) const
{
  auto copy = callback;
  if (!copy) return false;
  copy(std::move(t));
  return true;
}

}