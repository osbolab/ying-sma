#pragma once

namespace sma
{
struct MessageFlow {
  enum Weight { LIGHT, HEAVY };

private:
  MessageFlow();
  MessageFlow(MessageFlow const&);
  void operator=(MessageFlow const&);
};

struct ForwardPolicy {
  MessageFlow::Weight weight;
};
}
