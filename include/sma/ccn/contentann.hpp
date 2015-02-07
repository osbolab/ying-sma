#pragma once

#include <sma/ccn/contentmetadata.hpp>
#include <sma/networkdistance.hpp>

#include <sma/util/serial.hpp>

#include <vector>


namespace sma
{
struct ContentAnn {
  TRIVIALLY_SERIALIZABLE(ContentAnn, metadata, distance)

  std::vector<ContentMetadata> metadata;
  NetworkDistance distance;

  ContentAnn(std::vector<ContentMetadata> metadata,
             NetworkDistance distance = 0)
    : metadata(std::move(metadata))
    , distance(distance)
  {
  }

  ContentAnn(ContentAnn&&) = default;
  ContentAnn(ContentAnn const&) = default;

  ContentAnn& operator=(ContentAnn&&) = default;
  ContentAnn& operator=(ContentAnn const&) = default;
};
}
