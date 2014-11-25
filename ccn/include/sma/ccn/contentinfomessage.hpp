#pragma once

namespace sma
{
struct ContentInfoMessage {
  NodeId sender;

  ContentInfoMessage(NodeId sender)
    : sender(sender)
  {
  }

  template <typename Reader>
  ContentInfoMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;
};

template <typename Reader>
ContentInfoMessage::ContentInfoMessage(Reader* r)
{
}

template <typename Writer>
void ContentInfoMessage::write_fields(Writer* w) const
{
}
}
