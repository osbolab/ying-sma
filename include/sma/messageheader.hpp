#pragma once

#include <sma/nodeid.hpp>

#include <sma/util/reader.hpp>

#include <vector>

namespace sma
{
struct MessageHeader {
  NodeId sender;
  std::vector<NodeId> recipients;

  MessageHeader(NodeId sender)
    : sender(sender)
  {
  }

  MessageHeader(NodeId sender, std::vector<NodeId> recipients)
    : sender(sender)
    , recipients(std::move(recipients))
  {
  }

  template <typename... T>
  MessageHeader(Reader<T...>& r)
    : sender(r.template get<decltype(sender)>())
  {
    r.fill(recipients);
  }

  template <typename Writer>
  void write_fields(Writer& w) const
  {
    w << sender;
    w << recipients;
  }
};
}
