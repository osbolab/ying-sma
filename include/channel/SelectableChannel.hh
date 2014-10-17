#pragma once

#include "Channel.hh"
#include "Selector.hh"


namespace sma
{

template<class T>
class SelectableChannel : public Channel<T>
{
public:
  virtual void select(Selector<T>& selector) = 0;
};

}