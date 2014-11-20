#pragma once

#include <sma/device/inet_component.hpp>

#include <ns3/ptr.h>
#include <ns3/node.h>


namespace sma
{
class ns3_inet_component final : public inet_component
{
public:
  ns3_inet_component(ns3::Ptr<ns3::Node> this_node)
    : this_node(this_node)
  {
  }

  virtual std::unique_ptr<ns3_inet_socket> socket()
  {
    return std::make_unique<ns3_inet_socket>(this_node);
  }

private:
  ns3::Ptr<ns3::Node> this_node;
};
}
