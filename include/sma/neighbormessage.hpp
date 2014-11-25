#pragma once

#include <sma/messagetype.hpp>
#include <sma/nodeid.hpp>

namespace sma
{
struct NeighborMessage {
  static constexpr MessageType TYPE = 0;

  template <typename Reader>
  NeighborMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w);

  NodeId sender;
};


template <typename Reader>
NeighborMessage::NeighborMessage(Reader* r)
  : sender(r->template get<NodeId>())
{
}

template <typename Writer>
void NeighborMessage::write_fields(Writer* w)
{
  *w << sender;
}
}
