#pragma once

#include <sma/ccn/contenttype.hpp>

#include <sma/chrono>

namespace sma
{
struct RemoteInterest {
private:
  using clock = sma::chrono::system_clock;

public:
  using hop_count = std::uint8_t;

  /****************************************************************************
   * Serialized fields - Order matters!
   */
  ContentType type;
  hop_count hops;
  /***************************************************************************/

  RemoteInterest(ContentType type, hop_count hops)
    : type(type)
    , hops(hops)
  {
  }

  template <typename Reader>
  RemoteInterest(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w) const;

  bool closer_than(hop_count hops) { return this->hops < hops; }
};

template <typename Reader>
RemoteInterest::RemoteInterest(Reader* r)
  : type(r->template get<decltype(type)>())
  , hops(r->template get<hop_count>())
{
}

template <typename Writer>
void RemoteInterest::write_fields(Writer* w) const
{
  *w << type;
  *w << hops;
}
}
