#pragma once

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interest.hpp>
#include <sma/ccn/detail/remoteinterestentry.hpp>

#include <sma/io/log>

#include <map>

namespace sma
{
class CcnNode;
struct Message;
struct InterestMessage;

class InterestHelper
{
public:
  InterestHelper(CcnNode* node);
  void receive(Message&& msg, InterestMessage&& im);

  void insert_new(std::vector<ContentType> types);

private:
  void broadcast_interests(bool schedule_only = true);

  CcnNode* node;
  Logger log;

  std::map<ContentType, Interest> table;
  std::map<ContentType, detail::RemoteInterestEntry> r_table;
};
}
