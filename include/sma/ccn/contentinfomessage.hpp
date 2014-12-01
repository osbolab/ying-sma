#pragma once

#include <sma/ccn/contentinfo.hpp>
#include <sma/network_hops.hpp>

#include <sma/util/serial.hpp>

#include <ctime>

namespace sma
{
struct ContentInfoMessage {
  TRIVIALLY_SERIALIZABLE(ContentInfoMessage, hash, info)

  Hash hash;
  ContentInfo info;

  ContentInfoMessage(Hash hash, ContentInfo info)
    : hash(hash)
    , info(std::move(info))
  {
  }

  ContentInfoMessage(ContentInfoMessage&&) = default;
  ContentInfoMessage(ContentInfoMessage const&) = default;

  ContentInfoMessage& operator=(ContentInfoMessage&&) = default;
  ContentInfoMessage& operator=(ContentInfoMessage const&) = default;
};
}
