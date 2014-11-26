#include <sma/util/buffersource.hpp>

namespace sma
{
BufferSource::BufferSource(std::uint8_t const* src, std::size_t size)
  : is(&sb)
{
  sb.pubsetbuf(reinterpret_cast<char*>(const_cast<std::uint8_t*>(src)), size);
}
}
