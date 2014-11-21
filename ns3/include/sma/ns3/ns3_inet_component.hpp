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

  virtual std::unique_ptr<inet_socket> make_socket() override
  {
    return std::unique_ptr<inet_socket>(
        static_cast<inet_socket*>(new ns3_inet_socket(this_node)));
  }

  virtual std::unique_ptr<inet_channel> make_channel() override
  {
    return std::unique_ptr<inet_channel>(
        static_cast<inet_channel*>(new ns3_inet_channel()));
  }

private:
  ns3::Ptr<ns3::Node> this_node;
};
}
