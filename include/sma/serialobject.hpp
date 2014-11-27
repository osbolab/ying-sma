#pragma once

namespace sma
{
namespace detail
{
  struct ReadInto {
    template <typename T, typename Reader>
    static void receive(Reader&& reader, T&& into)
    {
      reader >> into;
    }
  };
}

struct SerialObject {
  TypeCode typecode;
  Buffer<uint16_t> data;

  template <typename T>
  T as()
  {
    return T(data.format<BinaryStreamReader>());
  }
};
}
