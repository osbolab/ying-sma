#pragma once

#include <sma/nodeid.hpp>
#include <sma/messagetype.hpp>
#include <sma/messagebody.hpp>

#include <cstdint>

namespace sma
{
struct Message;

struct NeighborMessage {
  using size_type = std::uint8_t;
  using body_type = MessageBody<size_type>;

  static constexpr MessageType TYPE = 0;

  static NeighborMessage read(std::uint8_t const* src, std::size_t size);

  NeighborMessage(NodeId sender);
  NeighborMessage(NodeId sender, body_type body);
  NeighborMessage(NeighborMessage&& r);
  NeighborMessage& operator=(NeighborMessage&& r);

  template <typename Reader>
  NeighborMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  Message to_message() const;

  // The order of these is important for serialization.

  NodeId sender;
  body_type body;
};

template <typename Reader>
NeighborMessage::NeighborMessage(Reader* r)
  : sender(r->template get<NodeId>())
  , body(r->template get<body_type>())
{
}

template <typename Writer>
void NeighborMessage::write_fields(Writer* w) const
{
  *w << sender;
  *w << body;
}
}
