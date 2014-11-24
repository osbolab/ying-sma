#pragma once

#include <sma/link/link.hpp>

#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/socket.h>

#include <cstdint>


namespace sma
{
class Ns3InetLink final : public Link
{
public:
  Ns3InetLink(ns3::Ptr<ns3::Node> this_node);
  Ns3InetLink(Ns3InetLink&& r);
  Ns3InetLink& operator=(Ns3InetLink&& r);
  virtual ~Ns3InetLink();

  virtual std::size_t read(void* dst, std::size_t size) override;
  virtual std::size_t write(void const* src, std::size_t size) override;

  virtual void close() override;

private:
  static constexpr auto NS3_FACTORY_TYPENAME = "ns3::UdpSocketFactory";
  static constexpr auto BCAST_ADDR = "10.1.255.255";
  static constexpr std::uint16_t BCAST_PORT = 9999;

  void bind();
  void packet_available(ns3::Ptr<ns3::Socket> s);

  ns3::Ptr<ns3::Socket> sock;
  ns3::Ptr<ns3::Packet> packet;
};
}
