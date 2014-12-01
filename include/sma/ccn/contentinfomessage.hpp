#pragma once

#include <sma/nodeid.hpp>
#include <sma/util/reader.hpp>

namespace sma
{
struct ContentInfoMessage {
  template <typename... T>
  ContentInfoMessage(Reader<T...>& r);

  template <typename Writer>
  void write_fields(Writer& w) const;
};

template <typename... T>
ContentInfoMessage::ContentInfoMessage(Reader<T...>& r)
{
}

template <typename Writer>
void ContentInfoMessage::write_fields(Writer& w) const
{
}
}
