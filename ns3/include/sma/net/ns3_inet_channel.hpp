#pragma once

#include <sma/net/inet_channel.hpp>

#include <ns3/ptr.h>

#include <vector>
#include <memory>


namespace ns3
{
class Packet;
}

namespace sma
{
class message;
class messenger;
class ns3_inet_socket;

class ns3_inet_channel final : public inet_channel
{
public:
  ns3_inet_channel();

  ns3_inet_channel(ns3_inet_channel&& rhs);
  ns3_inet_channel& operator=(ns3_inet_channel&& rhs);

  ~ns3_inet_channel();

  void add(std::unique_ptr<inet_socket> sock) override;

  void accept(message const& m) override;
  void close() override;

  // Target of callback from ns3_inet_socket when a packet arrives.
  void on_packet(ns3::Ptr<ns3::Packet> p);

private:
  std::size_t const SEND_BUFFER_SIZE = 1024;
  std::vector<std::unique_ptr<ns3_inet_socket>> socks;
};
}
