#include <sma/ccn/memorycontentstore.hpp>

#include <sma/ccn/contentdescriptor.hpp>


namespace sma
{
constexpr std::uint32_t StoredBlock::BLOCK_SIZE;


ContentDescriptor
MemoryContentStore::store(ContentType type, ContentName name, std::istream& is)
{
}
}
