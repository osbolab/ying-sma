#pragma once

#include <sma/ccn/contentmetadata.hpp>
#include <sma/networkdistance.hpp>

#include <sma/util/serial.hpp>

namespace sma
{
struct ContentAnn {
  TRIVIALLY_SERIALIZABLE(ContentAnn, metadata, distance)

  ContentMetadata metadata;
  NetworkDistance distance;

  ContentAnn(ContentMetadata metadata, NetworkDistance distance)
    : metadata(metadata)
    , distance(distance)
  {
  }

  ContentAnn(ContentAnn&&) = default;
  ContentAnn(ContentAnn const&) = default;

  ContentAnn& operator=(ContentAnn&&) = default;
  ContentAnn& operator=(ContentAnn const&) = default;
};
}
