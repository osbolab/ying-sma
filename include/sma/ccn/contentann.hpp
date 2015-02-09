#pragma once

#include <sma/ccn/contentmetadata.hpp>
#include <sma/networkdistance.hpp>

#include <sma/util/serial.hpp>

#include <vector>


namespace sma
{
struct ContentAnn {
  TRIVIALLY_SERIALIZABLE(ContentAnn, metadata, hops)

  std::vector<ContentMetadata> metadata;
  std::uint8_t hops;

  ContentAnn(std::vector<ContentMetadata> metadata,
             std::uint8_t hops = 0)
    : metadata(std::move(metadata))
    , hops(hops)
  {
  }

  ContentAnn(ContentAnn&&) = default;
  ContentAnn(ContentAnn const&) = default;

  ContentAnn& operator=(ContentAnn&&) = default;
  ContentAnn& operator=(ContentAnn const&) = default;
};
}
