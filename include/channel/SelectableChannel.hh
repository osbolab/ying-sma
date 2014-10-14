#ifndef SELECTABLECHANNEL_H_
#define SELECTABLECHANNEL_H_

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

#endif