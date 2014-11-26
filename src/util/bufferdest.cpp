#include <sma/util/bufferdest.hpp>

#include <cstring>
#include <cassert>

namespace sma
{
BufferDest::BufferDest()
  : os(&sb)
  , dynamic(true)
{
}

BufferDest::BufferDest(std::size_t size)
  : os(&sb)
  , owned(std::make_unique<char[]>(size))
{
  sb.pubsetbuf(owned.get(), size);
}
BufferDest::BufferDest(std::uint8_t* dst, std::size_t size)
  : os(&sb)
  , buf(reinterpret_cast<char*>(dst))
{
  sb.pubsetbuf(buf, size);
}

std::size_t BufferDest::read(std::uint8_t* dst, std::size_t size)
{
  auto pos = os.tellp();
  assert(pos >= 0);
  if (size < pos)
    size = pos;

  if (dynamic) {
    sb.pubseekpos(0, std::ios_base::in);
    sb.sgetn(reinterpret_cast<char*>(dst), size);
    sb.pubseekpos(size, std::ios_base::in);
  } else {
    std::memcpy(dst, buf, size);
  }

  return size;
}

std::size_t BufferDest::size() { return os.tellp(); }
}
