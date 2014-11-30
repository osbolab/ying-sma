#pragma once

#include <sma/nodeid.hpp>

namespace sma
{
struct ContentInfoMessage {
  template <typename Reader>
  ContentInfoMessage(Reader&& r);

  template <typename Writer>
  void write_fields(Writer&& w) const;
};

template <typename Reader>
ContentInfoMessage::ContentInfoMessage(Reader&& r)
{
}

template <typename Writer>
void ContentInfoMessage::write_fields(Writer&& w) const
{
}
}
