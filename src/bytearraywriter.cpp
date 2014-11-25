#include <sma/bytearraywriter.hpp>

#include <cassert>

namespace sma
{
ByteArrayWriter::ByteArrayWriter()
  : os(&sb)
  , dynamic(true)
{
}
ByteArrayWriter::ByteArrayWriter(std::size_t size)
  : os(&sb)
  , owned(std::make_unique<char[]>(size))
{
  sb.pubsetbuf(owned.get(), size);
}
ByteArrayWriter::ByteArrayWriter(std::uint8_t* dst, std::size_t size)
  : os(&sb)
  , dst(reinterpret_cast<char*>(dst))
{
  sb.pubsetbuf(this->dst, size);
}

std::uint8_t* ByteArrayWriter::data()
{
  if (dynamic) {
    auto pos = os.tellp();
    assert(pos >= 0);
    auto size = std::size_t(pos);
    owned = std::make_unique<char[]>(size);
    sb.pubseekpos(0, std::ios_base::in);
    sb.sgetn(owned.get(), size);
    sb.pubseekpos(size, std::ios_base::in);
    return reinterpret_cast<std::uint8_t*>(owned.get());
  }
  return reinterpret_cast<std::uint8_t*>(dst);
}

std::size_t ByteArrayWriter::size() { return os.tellp(); }
}
