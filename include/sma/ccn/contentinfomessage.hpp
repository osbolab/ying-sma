#pragma once

#include <sma/ccn/contentinfo.hpp>

#include <sma/util/serial.hpp>

#include <ctime>

namespace sma
{
struct ContentInfoMessage {
  TRIVIALLY_SERIALIZABLE(ContentInfoMessage, info, hops)

  using hop_count = std::uint16_t;

  ContentInfo info;
  hop_count hops = 0;

  ContentInfoMessage(ContentInfo info)
    : info(std::move(info))
  {
  }

  ContentInfoMessage(ContentInfoMessage&&) = default;
  ContentInfoMessage(ContentInfoMessage const&) = default;

  ContentInfoMessage& operator=(ContentInfoMessage&&) = default;
  ContentInfoMessage& operator=(ContentInfoMessage const&) = default;
};
}
