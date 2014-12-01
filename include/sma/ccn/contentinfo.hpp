#pragma once

#include <sma/nodeid.hpp>
#include <sma/network_hops.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>

#include <sma/chrono.hpp>

#include <ctime>
#include <functional>

namespace sma
{
struct ContentInfo {
  TRIVIALLY_SERIALIZABLE(ContentInfo, type, name, publisher, distance, blocks)

  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  /****************************
   * Serialized              */
  ContentType type;
  ContentName name;

  NodeId publisher;
  network_hops distance;

  std::vector<Hash> blocks;
  /**************************/

  /***************************
   * Transient              */
  time_point last_seen;
  /**************************/

  ContentInfo(ContentType type,
              ContentName name,
              NodeId publisher,
              std::vector<Hash> blocks = std::vector<Hash>(),
              network_hops distance = 0)
    : type(type)
    , name(name)
    , publisher(publisher)
    , distance(distance)
    , blocks(std::move(blocks))
  {
    touch();
  }

  void touch() { last_seen = clock::now(); }

  bool update(ContentInfo const& info)
  {
    touch();
    if (info.distance >= distance)
      return false;
    distance = info.distance;
    publisher = info.publisher;
    return true;
  }
};
}

namespace std
{
template <>
struct hash<sma::ContentInfo> {
  using argument_type = sma::ContentInfo;
  using result_type = size_t;

  result_type operator()(argument_type const& info) const;
};
}
