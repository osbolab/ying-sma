#pragma once

namespace sma
{
struct BlockRequestMessage {
  NodeId sender;

  BlockRequestMessage(NodeId sender)
    : sender(sender)
  {
  }

  template <typename Reader>
  BlockRequestMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;
};

template <typename Reader>
BlockRequestMessage::BlockRequestMessage(Reader* r)
{
}

template <typename Writer>
void BlockRequestMessage::write_fields(Writer* w) const
{
}
}
