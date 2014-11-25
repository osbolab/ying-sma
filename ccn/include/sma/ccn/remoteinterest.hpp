#pragma once

#include <sma/ccn/contenttype.hpp>

#include <sma/serial/vector.hpp>

namespace sma
{
struct RemoteInterest {
  using hop_count = std::uint8_t;
  using type_count = std::uint8_t;
  using type_vector = std::vector<ContentType>;

  /****************************************************************************
   * Serialized fields - Order matters!
   */
  hop_count hops;
  type_vector types;
  /***************************************************************************/

  RemoteInterest(hop_count hops, type_vector types)
    : hops(hops)
    , types(std::move(types))
  {
  }

  template <typename Reader>
  RemoteInterest(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

private:
  using vec_reader = VectorReader<ContentType, type_count>;
  using vec_writer = VectorWriter<ContentType, type_count>;
};

template <typename Reader>
RemoteInterest::RemoteInterest(Reader* r)
  : hops(r->template get<hop_count>())
  , types(vec_reader::read(r))
{
}

template <typename Writer>
void RemoteInterest::write_fields(Writer* w) const
{
  *w << hops;
  *w << vec_writer(&types);
}
}
