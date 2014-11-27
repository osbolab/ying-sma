#pragma once

#include <sma/nodeid.hpp>
#include <sma/messagetype.hpp>

namespace sma
{
struct ContentInfoMessage {
  static constexpr MessageType TYPE = 65;

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
