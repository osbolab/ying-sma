#pragma once

#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>
#include <sma/location.hpp>

#include <sma/util/serial.hpp>

#include <cstdint>


namespace sma
{
struct ContentMetadata {
  TRIVIALLY_SERIALIZABLE(ContentMetadata,
                         hash,
                         size,
                         block_size,
                         type,
                         name,
                         origin,
                         publisher,
                         publish_time)

  using size_type = std::uint32_t;

  Hash hash;
  size_type size;
  size_type block_size;

  ContentType type;
  ContentName name;
  Location origin;
  NodeId publisher;
  std::uint64_t publish_time;


  ContentMetadata(Hash hash,
                  size_type size,
                  size_type block_size,
                  ContentType type,
                  ContentName name,
                  Location origin,
                  NodeId publisher,
                  std::uint64_t publish_time)
    : hash(hash)
    , size(size)
    , block_size(block_size)
    , type(type)
    , name(name)
    , origin(origin)
    , publisher(publisher)
    , publish_time(publish_time)
  {
  }
};
}
