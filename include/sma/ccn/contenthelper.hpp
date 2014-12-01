#pragma once

#include <sma/io/log>

namespace sma
{
class CcnNode;

struct MessageHeader;
struct ContentInfoMessage;

class ContentHelper
{
public:
  ContentHelper(CcnNode& node);

  void receive(MessageHeader header, ContentInfoMessage msg);

private:
  CcnNode* node;
  Logger log;
};
}
