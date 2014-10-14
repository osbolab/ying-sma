#ifndef SMA_SINK_H_
#define SMA_SINK_H_

namespace sma
{

template<class T>
class Sink
{
public:
  virtual bool sink(const T& item) = 0;
};

}

#endif