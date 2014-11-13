#pragma once

#include <sma/socket.hpp>

#include <ns3/socket.h>
#include <ns3/ptr.h>


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
  ~ns3_socket();

  void bind(const socket_addr& address) override;
  void close() override;

  std::size_t recv(std::uint8_t* dst, std::size_t len) override;
  void send(const std::uint8_t* src,
            std::size_t len,
            const socket_addr& dest) override;

private:
  void throw_last_error();

  ns3::Ptr<ns3::Socket> sock{0};
};
}
