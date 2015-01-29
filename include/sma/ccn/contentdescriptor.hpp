#pragma once

#include <sma/nodeid.hpp>
#include <sma/networkdistance.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>

#include <sma/chrono.hpp>

#include <cstdint>
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
  TRIVIALLY_SERIALIZABLE(ContentDescriptor,
                         type,
                         name,
                         hash,
                         size,
                         block_size,
                         publisher,
                         distance)

  using size_type = std::uint32_t;
  using block_size_type = std::uint32_t;

  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  /****************************
   * Serialized              */
  ContentType type;
  ContentName name;

  Hash hash;

  size_type size;
  block_size_type block_size;

  NodeId publisher;
  NetworkDistance distance;
  /**************************/

  ContentDescriptor(Hash hash,
                    size_type size,
                    block_size_type block_size,
                    ContentType type,
                    ContentName name,
                    NodeId publisher,
                    NetworkDistance distance = 0)
    : hash(hash)
    , size(size)
    , block_size(block_size)
    , type(type)
    , name(name)
    , publisher(publisher)
    , distance(distance)
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
