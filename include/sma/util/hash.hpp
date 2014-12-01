#pragma once

#include <sma/util/serial.hpp>

#include <sma/util/detail/sha256.hpp>

#include <string>
#include <cstdint>
#include <cassert>
#include <functional>

namespace sma
{
struct Hash {
  friend struct Hasher;
  friend struct std::hash<Hash>;

  static constexpr std::size_t LENGTH = 32;

  Hash(Hash const&);
  Hash& operator=(Hash const&);

  DESERIALIZING_CTOR(Hash)
  {
    std::uint8_t length;
    GET_FIELD(length);
    assert(length == LENGTH);
    GET_BYTES(data, LENGTH);
  }

  SERIALIZER()
  {
    PUT_FIELD(std::uint8_t(LENGTH));
    PUT_BYTES(data, LENGTH);
  }

  int compare(Hash const&) const;

  bool operator==(Hash const& r) const { return compare(r) == 0; }
  bool operator!=(Hash const& r) const { return compare(r) != 0; }
  bool operator<(Hash const& r) const { return compare(r) < 0; }
  bool operator>(Hash const& r) const { return compare(r) > 0; }
  bool operator<=(Hash const& r) const { return compare(r) <= 0; }
  bool operator>=(Hash const& r) const { return compare(r) >= 0; }

  explicit operator std::string() const;

private:
  Hash() {}

  std::uint8_t data[LENGTH];
};

struct Hasher {
  Hasher() {}
  Hasher(void const* src, std::size_t size);
  Hasher(std::string const& s);

  void add(void const* src, std::size_t size);
  void add(std::string const& s);

  Hash digest();

private:
  SHA256 sha;
};
}

namespace std
{
template <>
struct hash<sma::Hash> {
  using argument_type = sma::Hash;
  using result_type = size_t;

  result_type operator()(argument_type const& a) const
  {
    size_t hash = 1;
    for (size_t i = 0; i < sma::Hash::LENGTH; ++i)
      hash *= 31 + a.data[i];
    return hash;
  }
};
}
