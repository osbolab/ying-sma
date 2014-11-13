#pragma once

#include <sma/channel.hpp>

#include <ns3/ptr.h>

#include <vector>

namespace ns3
{
  class Packet;
}

namespace sma
{
class message;
class messenger;
class ns3_socket;

class ns3_channel final : public channel
{
public:
  ns3_channel(ns3_socket* sock);
  ns3_channel(std::vector<ns3_socket*> socks);

  ns3_channel(ns3_channel&& rhs);
  ns3_channel& operator=(ns3_channel&& rhs);
  ns3_channel(const ns3_channel& rhs);
  ns3_channel& operator=(const ns3_channel& rhs);

  virtual ~ns3_channel();

  virtual void deliver_to(csink<message>* inbound) override;

  virtual void accept(const message& m) override;
  virtual void close() override;

  // Target of callback from ns3_socket when a packet arrives.
  void on_packet(ns3::Ptr<ns3::Packet> p);

private:
  const std::size_t SEND_BUFFER_SIZE = 1024;
  std::vector<ns3_socket*> socks;
};
}
