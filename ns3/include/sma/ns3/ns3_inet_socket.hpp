#pragma once

#include <sma/net/inet_socket.hpp>
#include <sma/net/inet_addr.hpp>
#include <sma/net/ns3_inet_channel.hpp>

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
class ns3_inet_socket final : public inet_socket
{
public:
  ns3_inet_socket(ns3::Ptr<ns3::Node> node);

  ns3_inet_socket(ns3_inet_socket&& rhs);
  ns3_inet_socket& operator=(ns3_inet_socket&& rhs);
  virtual ~ns3_inet_socket();

  virtual void bind(const socket_addr& address) override;
  virtual void close() override;

  virtual std::size_t recv(std::uint8_t* dst, std::size_t len) override;
  virtual void send(const std::uint8_t* src,
                    std::size_t len,
                    const socket_addr& dest) override;
  virtual void broadcast(const std::uint8_t* src, std::size_t len) override;

  // Set the inbox receiving channel
  void inbox(ns3_inet_channel* inbox) { ibx = inbox; }

private:
  // Target of callback from ns3::inet_socket when a packet arrives.
  void on_packet_available(ns3::Ptr<ns3::Socket> s);
  void throw_last_error();

  ns3::Ptr<ns3::Socket> sock{0};
  ns3::InetSocketAddress bind_addr;
  ns3_inet_channel* ibx;
};
}
