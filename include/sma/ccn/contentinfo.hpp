#pragma once

#include <sma/nodeid.hpp>
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
  TRIVIALLY_SERIALIZABLE(ContentInfo,
                         type,
                         name,
                         hash,
                         originating_node,
                         publishing_node,
                         creation_time,
                         publish_time)

  ContentType type;
  ContentName name;
  Hash hash;

  NodeId originating_node;
  NodeId publishing_node;

  std::time_t creation_time;
  std::time_t publish_time;


  ContentInfo(ContentType type, ContentName name, Hash hash, NodeId originating_node);

  bool operator==(ContentInfo const& r) const;
  bool operator!=(ContentInfo const& r) const;
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
