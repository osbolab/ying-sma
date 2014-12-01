#include <sma/ccn/contentinfo.hpp>
#include <sma/util/hash.hpp>

namespace sma
{

ContentInfo::ContentInfo(ContentType type,
                         ContentName name,
                         NodeId originating_node)
  : type(type)
  , name(name)
  , originating_node(originating_node)
  , publishing_node(originating_node)
{
  creation_time = publish_time
      = sma::chrono::system_clock::to_time_t(sma::chrono::system_clock::now());
}

bool ContentInfo::operator==(ContentInfo const& r) const
{
  return type == r.type && name == r.name
         && originating_node == r.originating_node
         && publishing_node == r.publishing_node;
}
bool ContentInfo::operator!=(ContentInfo const& r) const
{
  return !(*this == r);
}
}

namespace std
{
hash<sma::ContentInfo>::result_type hash<sma::ContentInfo>::
operator()(argument_type const& info) const
{
  sma::Hasher h;
  h.add(string(info.type));
  h.add(string(info.name));
  h.add(string(info.originating_node));
  h.add(string(info.publishing_node));
  return hash<sma::Hash>()(h.digest());
}
}
