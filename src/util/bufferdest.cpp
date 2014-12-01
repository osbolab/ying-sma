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

BufferDest::BufferDest(char* dst, std::size_t size)
  : os(&sb)
  , buf(dst)
{
  sb.pubsetbuf(buf, size);
}

BufferDest::BufferDest(std::uint8_t* dst, std::size_t size)
  : BufferDest(reinterpret_cast<char*>(dst), size)
{
}

std::size_t BufferDest::size() { return os.tellp(); }

void BufferDest::rewind() { sb.pubseekpos(0); }
}
