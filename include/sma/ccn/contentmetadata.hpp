#pragma once

#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/util/serial.hpp>


namespace sma
{
struct ContentMetadata {
  TRIVIALLY_SERIALIZABLE(
      ContentMetadata, hash, publisher, type, name, size, block_size)

  using size_type = std::uint32_t;

  Hash hash;
  NodeId publisher;

  ContentType type;
  ContentName name;

  size_type size;
  size_type block_size;


  ContentMetadata(Hash hash,
                  ContentType type,
                  ContentName name,
                  size_type size,
                  size_type block_size,
                  NodeId publisher)
    : hash(hash)
    , type(type)
    , name(name)
    , size(size)
    , block_size(block_size)
    , publisher(publisher)
  {
  }
};
}
