#pragma once

#include <sma/nodeid.hpp>
#include <sma/networkdistance.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>

#include <sma/chrono.hpp>

#include <ctime>
#include <functional>

namespace sma
{
//! Content metadata descriptor for a single content item.
/*! The descriptor must be sufficient to represent the content item's identity
 * across network boundaries and to facilitate retrieval across node boundaries
 * within the same application domain.
 */
struct ContentDescriptor {
  TRIVIALLY_SERIALIZABLE(
      ContentDescriptor, type, name, publisher, distance, blocks)

  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  /****************************
   * Serialized              */
  ContentType type;
  ContentName name;

  NodeId publisher;
  NetworkDistance distance;

  //! The block set naming each block
  std::vector<Hash> blocks;
  /**************************/

  ContentDescriptor(ContentType type,
                    ContentName name,
                    NodeId publisher,
                    std::vector<Hash> blocks = std::vector<Hash>(),
                    NetworkDistance distance = 0)
    : type(type)
    , name(name)
    , publisher(publisher)
    , distance(distance)
    , blocks(std::move(blocks))
  {
    touch();
  }

  bool update(ContentDescriptor const& info)
  {
    touch();
    if (info.distance >= distance)
      return false;

    distance = info.distance;
    publisher = info.publisher;
    return true;
  }

private:
  void touch() { last_seen = clock::now(); }

  /***************************
   * Transient              */
  time_point last_seen;
  /**************************/
};
}

namespace std
{
template <>
struct hash<sma::ContentDescriptor> {
  using argument_type = sma::ContentDescriptor;
  using result_type = size_t;

  result_type operator()(argument_type const& info) const;
};
}
