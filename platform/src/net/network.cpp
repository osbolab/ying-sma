#include <sma/net/network.hpp>

#include <sma/device/device.hpp>
#include <sma/device/inet_component.hpp>

#include <sma/unique_cast>


namespace sma
{
network::network(device* dev)
{
  auto inet = dev->try_get<inet_component>();
  assert(inet);

  // Create sockets to access this device's network interfaces
  auto sock = inet->make_socket();
  sock->bind(socket_address("0.0.0.0", 9999));

  // And wrap them in channels for the messaging layer to use
  auto chan = inet->make_channel();
  chan->add(sock);

  // For now we use the same inet interface for all traffic, but light_chan
  // should use something like BT/BLE where available.
  heavy_chan = light_chan = static_cast<channel*>(chan.get());
  channels.push_back(static_unique_cast<channel>(std::move(chan)));
}

network::network(network&& r)
  : channels(std::move(r.channels))
  , heavy_chan(r.heavy_chan)
  , light_chan(r.light_chan)
{
  r.heavy_chan = r.light_chan = nullptr;
}
network& network::operator=(network&& r)
{
  std::swap(channels, r.channels);
  std::swap(heavy_chan, r.heavy_chan);
  std::swap(light_chan, r.light_chan);
  return *this;
}
}
