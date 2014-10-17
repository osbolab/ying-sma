#pragma once

#include <memory>
#include <vector>

#include "Channel.hh"
#include "SelectableChannel.hh"

namespace sma
{

template<class T>
class SelectableChannel;

template<class T>
class Selector
{
public:
  virtual std::shared_ptr<Channel<T>> select() = 0;

private:
  std::vector<SelectableChannel<T>> channels;
};

}