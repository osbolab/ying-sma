#pragma once

namespace sma
{
class messenger;
class device;

struct context {
  friend class actor;

  context(device* dev, messenger* msgr)
    : dev(dev)
    , msgr(msgr)
  {
  }

  device* dev;

private:
  messenger* msgr;
};
}
