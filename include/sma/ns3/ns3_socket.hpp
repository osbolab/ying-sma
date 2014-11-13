#pragma once

#include <sma/socket.hpp>
#include <sma/inet_addr.hpp>
#include <sma/ns3/ns3_channel.hpp>

#include <ns3/ptr.h>
#include <ns3/socket.h>

#include <cstdint>


namespace ns3
{
class Node;
class TypeId;
}

namespace sma
{
class ns3_socket final : public socket
{
public:
  ns3_socket(ns3::TypeId socket_factory_tid, ns3::Ptr<ns3::Node> node);

  ns3_socket(ns3_socket&& rhs);
  ns3_socket& operator=(ns3_socket&& rhs);
  virtual ~ns3_socket();

  virtual void bind(const socket_addr& address) override;
  virtual void close() override;

  virtual std::size_t recv(std::uint8_t* dst, std::size_t len) override;
  virtual void send(const std::uint8_t* src,
                    std::size_t len,
                    const socket_addr& dest) override;
  virtual void broadcast(const std::uint8_t* src, std::size_t len) override;

  // Set the inbound receiving channel
  void receive_to(ns3_channel* inbound);

private:
  // Target of callback from ns3::Socket when a packet arrives.
  void on_packet(ns3::Ptr<ns3::Socket> s);
  void throw_last_error();

  ns3::Ptr<ns3::Socket> sock{0};
  ns3::InetSocketAddress bind_addr;
  ns3_channel* inbound{nullptr};
};
}
