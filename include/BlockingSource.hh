#ifndef BLOCKINGSOURCE_H_
#define BLOCKINGSOURCE_H_

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
#endif