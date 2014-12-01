#include <sma/util/hash.hpp>

#include <sstream>
#include <iomanip>
#include <cstring>

namespace sma
{

Hasher::Hasher(void const* src, std::size_t size)
{
  add(src, size);
}

Hasher::Hasher(std::string const& s)
{
  add(s);
}

void Hasher::add(void const* src, std::size_t size)
{
  sha.add(src, size);
}

void Hasher::add(std::string const& s)
{
  sha.add(s.c_str(), s.size());
}

Hash Hasher::digest()
{
  Hash hash;
  sha.getHash(hash.data);
  return hash;
}

Hash::Hash(Hash const& r) { std::memcpy(data, r.data, LENGTH); }

Hash& Hash::operator=(Hash const& r)
{
  std::memcpy(data, r.data, LENGTH);
  return *this;
}

int Hash::compare(Hash const& r) const
{
  for (std::size_t i = 0; i < LENGTH; ++i)
    if (data[i] > r.data[i])
      return 1;
    else if (data[i] < r.data[i])
      return -1;
  return 0;
}

Hash::operator std::string() const
{
  std::stringstream ss;
  for (std::size_t i = 0; i < LENGTH; ++i)
    ss << std::hex << std::setw(2) << std::setfill('0')
       << std::uint32_t(data[i]);
  return ss.str();
}
}
