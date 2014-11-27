#pragma once

namespace sma
{
struct BlockMessage {
  NodeId sender;

  BlockMessage(NodeId sender)
    : sender(sender)
  {
  }

  template <typename Reader>
  BlockMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;
};

template <typename Reader>
BlockMessage::BlockMessage(Reader* r)
{
}

template <typename Writer>
void BlockMessage::write_fields(Writer* w) const
{
}
}
