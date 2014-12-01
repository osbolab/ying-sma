#pragma once

#include <sma/ccn/content.hpp>
#include <sma/ccn/contentinfo.hpp>

#include <sma/chrono.hpp>
#include <sma/io/log>

#include <iosfwd>
#include <unordered_map>

namespace sma
{
class CcnNode;

struct ContentType;
struct ContentName;

struct MessageHeader;
struct ContentInfoMessage;

class ContentHelper : public Content
{
public:
  ContentHelper(CcnNode& node);

  void receive(MessageHeader header, ContentInfoMessage msg);

  virtual void
  publish(ContentType type, ContentName name, std::istream& is) override;

private:
  bool update_kct(Hash const& hash, ContentInfo const& info);

  CcnNode* node;
  Logger log;

  // Known Content Table
  std::unordered_map<Hash, ContentInfo> kct;
};
}
