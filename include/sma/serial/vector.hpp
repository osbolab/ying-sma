#pragma once

#include <vector>
#include <limits>
#include <cassert>

namespace sma
{
template <typename T, typename SizeT>
class VectorWriter
{
public:
  VectorWriter() {}
  VectorWriter(std::vector<T> const* v)
    : v(v)
  {
  }

  template <typename Writer>
  static void write(Writer* w, std::vector<T> const& v);

  template <typename Writer>
  void write_fields(Writer* w);

private:
  std::vector<T> const* v;
};




template <typename T, typename SizeT>
class VectorReader
{
public:
  template <typename Reader>
  static std::vector<T> read(Reader* r)
  {
    std::vector<T> v;
    auto size = r->template get<SizeT>();
    if (size == 0)
      return v;

    v.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
      v.emplace_back(r->template get<T>());
    return v;
  }
};



template <typename T, typename SizeT>
template <typename Writer>
void VectorWriter<T, SizeT>::write(Writer* w, std::vector<T> const& v)
{
  assert(v.size() < std::numeric_limits<SizeT>::max());
  *w << SizeT(v.size());
  for (auto& t : v)
    *w << v;
}

template <typename T, typename SizeT>
template <typename Writer>
void VectorWriter<T, SizeT>::write_fields(Writer* w)
{
  assert(v);
  write(w, *v);
}
}
