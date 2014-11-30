#pragma once

namespace sma
{
class LinkLayer;

class SendStrategy
{
public:
  SendStrategy(LinkLayer& llayer);

  virtual ~SendStrategy() {}

  virtual void notify() = 0;

protected:
  LinkLayer* llayer;
};
}
