#include <sma/util/buffersource.hpp>

namespace sma
{
BufferSource::BufferSource(char const* src, std::size_t size)
  : is(&sb)
{
  sb.pubsetbuf(const_cast<char*>(src), size);
}

BufferSource::BufferSource(std::uint8_t const* src, std::size_t size)
  : is(&sb)
{
  sb.pubsetbuf(reinterpret_cast<char*>(const_cast<std::uint8_t*>(src)), size);
}

void BufferSource::rewind() { sb.pubseekpos(0); }
}
