#pragma once

#include <sma/channel.hpp>
#include <memory>


namespace sma
{
  class device;

class network final
{
public:
  network(device* dev);
  network(network&& r);
  network& operator=(network&& r);

  channel* heavy_channel() { return heavy_chan; }
  channel* light_channel() { return light_chan; }

private:
  std::vector<std::unique_ptr<channel>> channels;
  channel* heavy_chan{nullptr};
  channel* light_chan{nullptr};
};
}
