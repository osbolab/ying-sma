#pragma once

#include <sma/ccn/contentmetadata.hpp>
#include <sma/networkdistance.hpp>

#include <sma/chrono.hpp>

#include <chrono>
#include <cstdint>


namespace sma
{
class CcnNode;
class ContentHelper;
class ContentStore;

class RemoteContent
{
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

public:
  RemoteContent(CcnNode& node,
                ContentHelper& helper,
                ContentStore& store,
                ContentMetadata metadata,
                NetworkDistance distance);

  ContentMetadata metadata() const { return meta; }

  void fetch_block(std::uint32_t index, std::chrono::milliseconds timeout);

  bool saw(NetworkDistance distance);

private:
  void touch() { last_seen = clock::now(); }

  CcnNode& node;
  ContentHelper& helper;
  ContentStore& store;

  ContentMetadata meta;
  time_point last_seen;
  NetworkDistance dist;
};
}
